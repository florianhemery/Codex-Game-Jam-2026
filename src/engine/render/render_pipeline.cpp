/// \file render_pipeline.cpp
/// \brief Implementation du pipeline : passe d'ombres ortho depuis le soleil,
///        passe scene HDR (dome de ciel + shader eclaire), passe post.

#include "engine/render/render_pipeline.h"

#include "raymath.h"
#include "rlgl.h"

#include <array>
#include <cmath>
#include <filesystem>

namespace racer::engine {

namespace {

constexpr int kShadowRes = 2048;       // resolution de la shadow map
constexpr float kShadowExtent = 130.0f; // etendue ortho autour de camera.target
constexpr float kShadowDist = 260.0f;  // recul de la camera lumiere
constexpr float kSkyRadius = 450.0f;   // rayon du dome (far plane raylib = 1000)
constexpr int kShadowSlot = 15;        // unite de texture reservee a la shadow map

/// Emplacement d'uniform avec repli sur la syntaxe tableau "nom[0]".
int LocOrArray(const Shader& shader, const char* name, const char* arrayName)
{
    int loc = GetShaderLocation(shader, name);
    if (loc == -1) loc = GetShaderLocation(shader, arrayName);
    return loc;
}

/// Shader par defaut raylib (pour neutraliser un materiau avant UnloadModel).
Shader DefaultShader()
{
    Shader shader{};
    shader.id = rlGetShaderIdDefault();
    shader.locs = rlGetShaderLocsDefault();
    return shader;
}

} // namespace

RenderPipeline::RenderPipeline(int screenWidth, int screenHeight, const char* shaderDir)
    : width_(screenWidth), height_(screenHeight)
{
    const std::string dir = ResolveShaderDir(shaderDir);
    TraceLog(LOG_INFO, "RENDER: dossier shaders: %s", dir.c_str());

    shadowRT_ = device_.CreateRenderTarget({kShadowRes, kShadowRes, RhiFormat::DEPTH24, true});
    sceneRT_ = device_.CreateRenderTarget({width_, height_, RhiFormat::RGBA16F, true});

    // Le post-process echantillonne pres des bords (blur radial, aberration) :
    // sans clamp, le wrap REPEAT ferait fuir le bord oppose dans l'image.
    if (const RenderTexture2D* hdr = device_.GetRenderTexture(sceneRT_)) {
        SetTextureWrap(hdr->texture, TEXTURE_WRAP_CLAMP);
    }
    if (const RenderTexture2D* shadow = device_.GetRenderTexture(shadowRT_)) {
        SetTextureWrap(shadow->depth, TEXTURE_WRAP_CLAMP);
    }

    lit_ = &watcher_.RegisterShader("lit", dir + "/lit.vs", dir + "/lit.fs");
    sky_ = &watcher_.RegisterShader("sky", dir + "/sky.vs", dir + "/sky.fs");
    post_ = &watcher_.RegisterShader("post", "", dir + "/post.fs"); // vertex par defaut raylib
    watcher_.SetOnReload([this](const std::string&, Shader&) { RefreshLocations(); });

    skyDome_ = LoadModelFromMesh(GenMeshSphere(1.0f, 24, 48));
    skyDomeLoaded_ = true;

    RefreshLocations();
    SetAmbiance(Ambiance::Midi);
}

RenderPipeline::~RenderPipeline()
{
    if (skyDomeLoaded_ && IsWindowReady())
    {
        // Le shader du dome appartient au ShaderWatcher : on le neutralise
        // pour eviter une double liberation par UnloadModel.
        skyDome_.materials[0].shader = DefaultShader();
        UnloadModel(skyDome_);
        skyDomeLoaded_ = false;
    }
}

const AmbianceParams& RenderPipeline::ParamsFor(Ambiance a)
{
    // Presets soignes : ce sont eux qui definissent l'identite visuelle.
    static const std::array<AmbianceParams, 4> kPresets = {{
        // Midi : soleil haut legerement chaud, ciel bleu franc, brouillard leger.
        {
            /*sunDir*/        {-0.603f, -0.784f, 0.151f},
            /*sunColor*/      {1.90f, 1.78f, 1.55f},
            /*skyAmbient*/    {0.30f, 0.36f, 0.47f},
            /*groundAmbient*/ {0.20f, 0.19f, 0.17f},
            /*skyZenith*/     {0.13f, 0.34f, 0.78f},
            /*skyHorizon*/    {0.66f, 0.80f, 0.94f},
            /*cloudCoverage*/ 0.32f,
            /*cloudTint*/     {1.00f, 1.00f, 1.02f},
            /*fogColor*/      {0.62f, 0.73f, 0.88f},
            /*fogDensity*/    0.0060f,
            /*exposure*/      0.85f,
            /*gradeTint*/     {1.0f, 1.0f, 1.0f},
            /*saturation*/    1.15f,
            /*vignette*/      0.18f,
            /*headlights*/    false,
            /*stars*/         false,
        },
        // Aube doree : soleil bas tres chaud, ciel peche/or, brume dense doree.
        {
            /*sunDir*/        {0.683f, -0.259f, 0.683f},
            /*sunColor*/      {3.0f, 1.85f, 0.95f},
            /*skyAmbient*/    {0.52f, 0.44f, 0.40f},
            /*groundAmbient*/ {0.30f, 0.24f, 0.19f},
            /*skyZenith*/     {0.34f, 0.44f, 0.66f},
            /*skyHorizon*/    {1.05f, 0.62f, 0.32f},
            /*cloudCoverage*/ 0.45f,
            /*cloudTint*/     {1.10f, 0.82f, 0.60f},
            /*fogColor*/      {0.92f, 0.70f, 0.46f},
            /*fogDensity*/    0.009f,
            /*exposure*/      1.0f,
            /*gradeTint*/     {1.07f, 1.00f, 0.90f},
            /*saturation*/    1.12f,
            /*vignette*/      0.24f,
            /*headlights*/    false,
            /*stars*/         false,
        },
        // Crepuscule : soleil rasant orange-rose, zenith pourpre, phares allumes.
        {
            /*sunDir*/        {0.705f, -0.070f, 0.705f},
            /*sunColor*/      {2.6f, 1.05f, 0.62f},
            /*skyAmbient*/    {0.30f, 0.22f, 0.38f},
            /*groundAmbient*/ {0.14f, 0.11f, 0.14f},
            /*skyZenith*/     {0.16f, 0.08f, 0.30f},
            /*skyHorizon*/    {0.98f, 0.40f, 0.34f},
            /*cloudCoverage*/ 0.42f,
            /*cloudTint*/     {0.62f, 0.36f, 0.46f},
            /*fogColor*/      {0.40f, 0.24f, 0.38f},
            /*fogDensity*/    0.009f,
            /*exposure*/      0.95f,
            /*gradeTint*/     {1.06f, 0.94f, 1.05f},
            /*saturation*/    1.04f,
            /*vignette*/      0.34f,
            /*headlights*/    true,
            /*stars*/         true,
        },
        // Orage : soleil quasi absent froid, couverture 1.0, brouillard dense.
        {
            /*sunDir*/        {-0.305f, -0.927f, -0.218f},
            /*sunColor*/      {0.30f, 0.34f, 0.40f},
            /*skyAmbient*/    {0.30f, 0.33f, 0.37f},
            /*groundAmbient*/ {0.13f, 0.14f, 0.15f},
            /*skyZenith*/     {0.10f, 0.11f, 0.13f},
            /*skyHorizon*/    {0.20f, 0.22f, 0.25f},
            /*cloudCoverage*/ 1.0f,
            /*cloudTint*/     {0.13f, 0.14f, 0.17f},
            /*fogColor*/      {0.24f, 0.27f, 0.30f},
            /*fogDensity*/    0.016f,
            /*exposure*/      0.72f,
            /*gradeTint*/     {0.90f, 0.96f, 1.05f},
            /*saturation*/    0.80f,
            /*vignette*/      0.38f,
            /*headlights*/    true,
            /*stars*/         false,
        },
    }};
    return kPresets[static_cast<std::size_t>(a)];
}

void RenderPipeline::SetAmbiance(Ambiance a)
{
    ambiance_ = a;
    params_ = ParamsFor(a);
}

Shader RenderPipeline::LitShader() const
{
    return lit_->Get();
}

void RenderPipeline::ClearLights()
{
    lightCount_ = 0;
}

void RenderPipeline::AddLight(Vector3 position, Vector3 colorIntensity)
{
    if (lightCount_ >= kMaxLights) return;
    lightsPos_[lightCount_] = position;
    lightsColor_[lightCount_] = colorIntensity;
    ++lightCount_;
}

void RenderPipeline::PollShaderReload()
{
    watcher_.Poll();
}

void RenderPipeline::Resize(int screenWidth, int screenHeight)
{
    if (screenWidth < 1 || screenHeight < 1) return;
    if (screenWidth == width_ && screenHeight == height_) return;

    device_.DestroyRenderTarget(sceneRT_);
    width_ = screenWidth;
    height_ = screenHeight;
    sceneRT_ = device_.CreateRenderTarget({width_, height_, RhiFormat::RGBA16F, true});

    if (const RenderTexture2D* hdr = device_.GetRenderTexture(sceneRT_)) {
        SetTextureWrap(hdr->texture, TEXTURE_WRAP_CLAMP);
    }
}

std::string RenderPipeline::ResolveShaderDir(const char* shaderDir) const
{
    if (shaderDir != nullptr) return shaderDir;

    // Le jeu tourne depuis build/Debug/ : on remonte jusqu'a trouver les assets.
    const std::array<const char*, 4> candidates = {
        "assets/shaders", "../assets/shaders", "../../assets/shaders", "../../../assets/shaders"};
    for (const char* candidate : candidates)
    {
        std::error_code ec;
        if (std::filesystem::exists(std::filesystem::path(candidate) / "lit.fs", ec)) return candidate;
    }
    TraceLog(LOG_WARNING, "RENDER: dossier shaders introuvable, repli sur assets/shaders");
    return "assets/shaders";
}

void RenderPipeline::RefreshLocations()
{
    const Shader& lit = lit_->Get();
    litLocs_.viewPos = GetShaderLocation(lit, "viewPos");
    litLocs_.sunDir = GetShaderLocation(lit, "sunDir");
    litLocs_.sunColor = GetShaderLocation(lit, "sunColor");
    litLocs_.skyAmbient = GetShaderLocation(lit, "skyAmbient");
    litLocs_.groundAmbient = GetShaderLocation(lit, "groundAmbient");
    litLocs_.fogColor = GetShaderLocation(lit, "fogColor");
    litLocs_.fogDensity = GetShaderLocation(lit, "fogDensity");
    litLocs_.lightVP = GetShaderLocation(lit, "lightVP");
    litLocs_.shadowMap = GetShaderLocation(lit, "shadowMap");
    litLocs_.shadowTexel = GetShaderLocation(lit, "shadowTexel");
    litLocs_.lightsPos = LocOrArray(lit, "lightsPos", "lightsPos[0]");
    litLocs_.lightsColor = LocOrArray(lit, "lightsColor", "lightsColor[0]");
    litLocs_.lightsCount = GetShaderLocation(lit, "lightsCount");

    const Shader& sky = sky_->Get();
    skyLocs_.sunDir = GetShaderLocation(sky, "sunDir");
    skyLocs_.sunColor = GetShaderLocation(sky, "sunColor");
    skyLocs_.zenith = GetShaderLocation(sky, "skyZenith");
    skyLocs_.horizon = GetShaderLocation(sky, "skyHorizon");
    skyLocs_.coverage = GetShaderLocation(sky, "cloudCoverage");
    skyLocs_.tint = GetShaderLocation(sky, "cloudTint");
    skyLocs_.fogColor = GetShaderLocation(sky, "fogColor");
    skyLocs_.time = GetShaderLocation(sky, "time");
    skyLocs_.stars = GetShaderLocation(sky, "starsOn");

    const Shader& post = post_->Get();
    postLocs_.exposure = GetShaderLocation(post, "exposure");
    postLocs_.gradeTint = GetShaderLocation(post, "gradeTint");
    postLocs_.saturation = GetShaderLocation(post, "saturation");
    postLocs_.vignette = GetShaderLocation(post, "vignette");
    postLocs_.aberration = GetShaderLocation(post, "aberration");
    postLocs_.grainAmount = GetShaderLocation(post, "grainAmount");
    postLocs_.time = GetShaderLocation(post, "time");
    postLocs_.speedBlur = GetShaderLocation(post, "speedBlur");

    // Le dome porte toujours le shader courant (l'id change au hot-reload).
    if (skyDomeLoaded_) skyDome_.materials[0].shader = sky_->Get();
}

void RenderPipeline::BindShadowMapTexture(unsigned int textureId)
{
    if (litLocs_.shadowMap == -1) return;
    const Shader& lit = lit_->Get();
    rlEnableShader(lit.id);
    int slot = kShadowSlot;
    rlActiveTextureSlot(kShadowSlot);
    rlEnableTexture(textureId);
    rlSetUniform(litLocs_.shadowMap, &slot, SHADER_UNIFORM_INT, 1);
    rlActiveTextureSlot(0);
}

void RenderPipeline::UploadLitUniforms(const Camera3D& camera)
{
    const Shader& lit = lit_->Get();
    SetShaderValue(lit, litLocs_.viewPos, &camera.position, SHADER_UNIFORM_VEC3);
    SetShaderValue(lit, litLocs_.sunDir, &params_.sunDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(lit, litLocs_.sunColor, &params_.sunColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(lit, litLocs_.skyAmbient, &params_.skyAmbient, SHADER_UNIFORM_VEC3);
    SetShaderValue(lit, litLocs_.groundAmbient, &params_.groundAmbient, SHADER_UNIFORM_VEC3);
    SetShaderValue(lit, litLocs_.fogColor, &params_.fogColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(lit, litLocs_.fogDensity, &params_.fogDensity, SHADER_UNIFORM_FLOAT);
    SetShaderValueMatrix(lit, litLocs_.lightVP, lightVP_);

    const float texel = 1.0f/static_cast<float>(kShadowRes);
    SetShaderValue(lit, litLocs_.shadowTexel, &texel, SHADER_UNIFORM_FLOAT);

    SetShaderValueV(lit, litLocs_.lightsPos, lightsPos_, SHADER_UNIFORM_VEC3, kMaxLights);
    SetShaderValueV(lit, litLocs_.lightsColor, lightsColor_, SHADER_UNIFORM_VEC3, kMaxLights);
    SetShaderValue(lit, litLocs_.lightsCount, &lightCount_, SHADER_UNIFORM_INT);
}

void RenderPipeline::UploadSkyUniforms(float time)
{
    const Shader& sky = sky_->Get();
    SetShaderValue(sky, skyLocs_.sunDir, &params_.sunDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(sky, skyLocs_.sunColor, &params_.sunColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(sky, skyLocs_.zenith, &params_.skyZenith, SHADER_UNIFORM_VEC3);
    SetShaderValue(sky, skyLocs_.horizon, &params_.skyHorizon, SHADER_UNIFORM_VEC3);
    SetShaderValue(sky, skyLocs_.coverage, &params_.cloudCoverage, SHADER_UNIFORM_FLOAT);
    SetShaderValue(sky, skyLocs_.tint, &params_.cloudTint, SHADER_UNIFORM_VEC3);
    SetShaderValue(sky, skyLocs_.fogColor, &params_.fogColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(sky, skyLocs_.time, &time, SHADER_UNIFORM_FLOAT);
    const int stars = params_.stars ? 1 : 0;
    SetShaderValue(sky, skyLocs_.stars, &stars, SHADER_UNIFORM_INT);
}

void RenderPipeline::UploadPostUniforms(float time, const PostParams& post)
{
    const float speed = Clamp(post.speedRatio, 0.0f, 1.0f);
    const float blur = speed*(post.nitro ? 1.4f : 1.0f);
    const float aberration = 0.0010f + 0.0035f*speed + (post.nitro ? 0.0020f : 0.0f);
    const float grain = 0.028f;

    const Shader& sh = post_->Get();
    SetShaderValue(sh, postLocs_.exposure, &params_.exposure, SHADER_UNIFORM_FLOAT);
    SetShaderValue(sh, postLocs_.gradeTint, &params_.gradeTint, SHADER_UNIFORM_VEC3);
    SetShaderValue(sh, postLocs_.saturation, &params_.saturation, SHADER_UNIFORM_FLOAT);
    SetShaderValue(sh, postLocs_.vignette, &params_.vignette, SHADER_UNIFORM_FLOAT);
    SetShaderValue(sh, postLocs_.aberration, &aberration, SHADER_UNIFORM_FLOAT);
    SetShaderValue(sh, postLocs_.grainAmount, &grain, SHADER_UNIFORM_FLOAT);
    SetShaderValue(sh, postLocs_.time, &time, SHADER_UNIFORM_FLOAT);
    SetShaderValue(sh, postLocs_.speedBlur, &blur, SHADER_UNIFORM_FLOAT);
}

void RenderPipeline::DrawSkyDome(const Camera3D& camera)
{
    // Dome vu de l'interieur, sans ecriture de profondeur : dessine en
    // premier, la scene le recouvre ensuite naturellement.
    rlDisableBackfaceCulling();
    rlDisableDepthMask();
    DrawModel(skyDome_, camera.position, kSkyRadius, WHITE);
    rlEnableBackfaceCulling();
    rlEnableDepthMask();
}

void RenderPipeline::Frame(const Camera3D& camera,
                           const std::function<void()>& drawShadowCasters,
                           const std::function<void()>& drawLitScene,
                           const std::function<void()>& drawUnlitInScene,
                           const PostParams& post)
{
    const float time = static_cast<float>(GetTime());

    // --- Passe 1 : ombres (profondeur seule, camera ortho depuis le soleil) ---
    Camera3D lightCam{};
    lightCam.position = Vector3Subtract(camera.target, Vector3Scale(params_.sunDir, kShadowDist));
    lightCam.target = camera.target;
    lightCam.up = (std::fabs(params_.sunDir.y) > 0.99f) ? Vector3{0.0f, 0.0f, 1.0f}
                                                        : Vector3{0.0f, 1.0f, 0.0f};
    lightCam.fovy = kShadowExtent;
    lightCam.projection = CAMERA_ORTHOGRAPHIC;

    // La shadow map est attachee au FBO pendant cette passe : on met la
    // texture blanche par defaut sur son slot pour eviter tout feedback.
    BindShadowMapTexture(rlGetTextureIdDefault());

    device_.BeginRenderTarget(shadowRT_);
    ClearBackground(WHITE);
    BeginMode3D(lightCam);
    // Matrices exactes de la passe, relues depuis rlgl.
    lightVP_ = MatrixMultiply(rlGetMatrixModelview(), rlGetMatrixProjection());
    rlSetCullFace(RL_CULL_FACE_FRONT); // reduit l'acne sur les volumes fermes
    BeginShaderMode(lit_->Get());
    if (drawShadowCasters) drawShadowCasters();
    EndShaderMode();
    rlSetCullFace(RL_CULL_FACE_BACK);
    EndMode3D();
    device_.EndRenderTarget();

    // --- Passe 2 : scene HDR (dome + scene eclairee + effets non eclaires) ---
    UploadLitUniforms(camera);
    UploadSkyUniforms(time);

    const RenderTexture2D* shadowTex = device_.GetRenderTexture(shadowRT_);
    BindShadowMapTexture(shadowTex != nullptr ? shadowTex->depth.id : rlGetTextureIdDefault());

    device_.BeginRenderTarget(sceneRT_);
    ClearBackground(BLACK);
    BeginMode3D(camera);
    DrawSkyDome(camera);
    BeginShaderMode(lit_->Get());
    if (drawLitScene) drawLitScene();
    EndShaderMode();
    if (drawUnlitInScene) drawUnlitInScene();
    EndMode3D();
    device_.EndRenderTarget();

    // --- Passe 3 : post-process HDR -> ecran ---
    UploadPostUniforms(time, post);

    const RenderTexture2D* hdr = device_.GetRenderTexture(sceneRT_);
    if (hdr != nullptr)
    {
        const Rectangle src{0.0f, 0.0f, static_cast<float>(hdr->texture.width),
                            -static_cast<float>(hdr->texture.height)};
        BeginShaderMode(post_->Get());
        DrawTextureRec(hdr->texture, src, Vector2{0.0f, 0.0f}, WHITE);
        EndShaderMode();
    }
}

} // namespace racer::engine

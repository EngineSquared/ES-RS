#include "LoadCourse.hpp"

#include "Entity.hpp"
#include "Mesh.hpp"
#include "OpenGL.hpp"
#include "Object.hpp"
#include "RigidBody3D.hpp"

#include "JoltPhysics.hpp"
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <filesystem>
#include <iostream>
#include <string>

void LoadCourseModels(ES::Engine::Core &core, const glm::vec3 &position, const glm::quat &rotation, const glm::vec3 &size)
{
    namespace fs = std::filesystem;

    const std::string directoryPath = "asset/course_model/";
    auto &textureManager = core.GetResource<ES::Plugin::OpenGL::Resource::TextureManager>();

    for (const auto &entry : fs::directory_iterator(directoryPath))
    {
        if (!entry.is_regular_file())
            continue;

        const std::string filePath = entry.path().string();
        if (entry.path().extension() != ".obj")
            continue;

        const std::string filename = entry.path().stem().string();

        ES::Plugin::Object::Component::Mesh objMesh;
        if (!ES::Plugin::Object::Resource::OBJLoader::loadModel(
            filePath,
            objMesh.vertices,
            objMesh.normals,
            objMesh.texCoords,
            objMesh.indices
        )) {
            ES::Utils::Log::Error(fmt::format("Failed to load model: {}", filePath));
            continue;
        }

        ES::Engine::Entity entity = core.CreateEntity();
        entity.AddComponent<ES::Plugin::Object::Component::Mesh>(core, objMesh);
        entity.AddComponent<ES::Plugin::Object::Component::Transform>(core, position, size, rotation);

        std::string textureName = "tex_" + filename;
        if (textureManager.Contains(entt::hashed_string{textureName.c_str()})) {
            entity.AddComponent<ES::Plugin::OpenGL::Component::TextureHandle>(core, textureName);
            ES::Utils::Log::Info(fmt::format("Course OBJ {} loaded with texture {}", filename, textureName));
        }
        else {
            entity.AddComponent<ES::Plugin::OpenGL::Component::TextureHandle>(core, "tex_PLACEHOLDER");
            ES::Utils::Log::Error(fmt::format("No texture found for model: {}", filename));
        }

        entity.AddComponent<ES::Plugin::OpenGL::Component::ShaderHandle>(core, "texture");
        entity.AddComponent<ES::Plugin::OpenGL::Component::MaterialHandle>(core, "level");
        entity.AddComponent<ES::Plugin::OpenGL::Component::ModelHandle>(core, fmt::format("level_{}", filename).c_str());
    }
    LoadCourseCollision(core, size);
}

void LoadCourseCollision(ES::Engine::Core &core, const glm::vec3 &size)
{
    const std::string colPath = "asset/collisions/level_collision.obj";
    ES::Engine::Entity entity = core.CreateEntity();
    ES::Plugin::Object::Component::Mesh objMesh;

    if (!ES::Plugin::Object::Resource::OBJLoader::loadModel(
        colPath,
        objMesh.vertices,
        objMesh.normals,
        objMesh.texCoords,
        objMesh.indices
    )) {
        ES::Utils::Log::Error(fmt::format("Failed to load model: {}", colPath));
    }

    JPH::VertexList vertexList;
    vertexList.reserve(objMesh.vertices.size());
    for (const auto &v : objMesh.vertices) {
        vertexList.emplace_back(v.x * size.x, v.y * size.y, v.z * size.z);
    }

    JPH::IndexedTriangleList triangleList;
    for (size_t i = 0; i + 2 < objMesh.indices.size(); i += 3) {
        triangleList.emplace_back(
            static_cast<uint32_t>(objMesh.indices[i]),
            static_cast<uint32_t>(objMesh.indices[i + 1]),
            static_cast<uint32_t>(objMesh.indices[i + 2])
        );
    }

    auto meshShapeSettings = std::make_shared<JPH::MeshShapeSettings>(vertexList, triangleList);

    entity.AddComponent<ES::Plugin::Physics::Component::RigidBody3D>(core, meshShapeSettings, JPH::EMotionType::Static, ES::Plugin::Physics::Utils::Layers::NON_MOVING);
    ES::Utils::Log::Info("Loaded course collision");
}

void LoadCourseTextures(ES::Engine::Core &core)
{
    auto &textureManager = core.GetResource<ES::Plugin::OpenGL::Resource::TextureManager>();

    // Placeholder texture in case a loading fails
    textureManager.Add(entt::hashed_string{"tex_PLACEHOLDER"}, "asset/textures/placeholder.png");

    // Using the placeholder texture for these as the model doesn't contains any texture on the mesh
    textureManager.Add(entt::hashed_string{"tex_CEMSTAIRS"}, "asset/textures/placeholder.png");
    textureManager.Add(entt::hashed_string{"tex_GRLTGLOW"}, "asset/textures/placeholder.png");
    textureManager.Add(entt::hashed_string{"tex_GRNDWALL"}, "asset/textures/placeholder.png");
    textureManager.Add(entt::hashed_string{"tex_PITENTB"}, "asset/textures/placeholder.png");
    textureManager.Add(entt::hashed_string{"tex_PITTENTA"}, "asset/textures/placeholder.png");
    textureManager.Add(entt::hashed_string{"tex_PTENTC"}, "asset/textures/placeholder.png");
    textureManager.Add(entt::hashed_string{"tex_PTENTD"}, "asset/textures/placeholder.png");
    textureManager.Add(entt::hashed_string{"tex_ROOFTOP"}, "asset/textures/placeholder.png");
    textureManager.Add(entt::hashed_string{"tex_TVAN"}, "asset/textures/placeholder.png");
    textureManager.Add(entt::hashed_string{"tex_WALGGG"}, "asset/textures/placeholder.png");

    textureManager.Add(entt::hashed_string{"tex_AIRCON"}, "asset/textures/AIRCON.png");
    textureManager.Add(entt::hashed_string{"tex_AIRCONOLD"}, "asset/textures/AIRCONOLD.png");
    textureManager.Add(entt::hashed_string{"tex_BARRELA"}, "asset/textures/BARRELA.png");
    textureManager.Add(entt::hashed_string{"tex_BILLBRDA"}, "asset/textures/BILLBRDA.png");
    textureManager.Add(entt::hashed_string{"tex_BILLBRDBK"}, "asset/textures/BILLBRDBK.png");
    textureManager.Add(entt::hashed_string{"tex_BRDGSDA"}, "asset/textures/BRDGSDA.png");
    textureManager.Add(entt::hashed_string{"tex_BRNCHA"}, "asset/textures/BRNCHA.png");
    textureManager.Add(entt::hashed_string{"tex_BRNCHB"}, "asset/textures/BRNCHB.png");
    textureManager.Add(entt::hashed_string{"tex_BRNCHC"}, "asset/textures/BRNCHC.png");
    textureManager.Add(entt::hashed_string{"tex_BULBGLO"}, "asset/textures/BULBGLO.png");
    textureManager.Add(entt::hashed_string{"tex_BULBGLO_LONG"}, "asset/textures/BULBGLO_LONG.png");
    textureManager.Add(entt::hashed_string{"tex_CEILINGA"}, "asset/textures/CEILINGA.png");
    textureManager.Add(entt::hashed_string{"tex_CEMY"}, "asset/textures/CEMY.png");
    textureManager.Add(entt::hashed_string{"tex_CEMZ"}, "asset/textures/CEMZ.png");
    textureManager.Add(entt::hashed_string{"tex_CMWLLA"}, "asset/textures/CMWLLA.png");
    textureManager.Add(entt::hashed_string{"tex_CNUMB"}, "asset/textures/CNUMB.png");
    textureManager.Add(entt::hashed_string{"tex_CONE"}, "asset/textures/CONE.png");
    textureManager.Add(entt::hashed_string{"tex_CWORKERA"}, "asset/textures/CWORKERA.png");
    textureManager.Add(entt::hashed_string{"tex_EBOXA"}, "asset/textures/EBOXA.png");
    textureManager.Add(entt::hashed_string{"tex_FENCEA"}, "asset/textures/FENCEA.png");
    textureManager.Add(entt::hashed_string{"tex_FENCEATOP"}, "asset/textures/FENCEATOP.png");
    textureManager.Add(entt::hashed_string{"tex_FENCEB"}, "asset/textures/FENCEB.png");
    textureManager.Add(entt::hashed_string{"tex_FENCEC"}, "asset/textures/FENCEC.png");
    textureManager.Add(entt::hashed_string{"tex_FLAGLINEA"}, "asset/textures/FLAGLINEA.png");
    textureManager.Add(entt::hashed_string{"tex_FPOSTWDA"}, "asset/textures/FPOSTWDA.png");
    textureManager.Add(entt::hashed_string{"tex_FPOSTWDB"}, "asset/textures/FPOSTWDB.png");
    textureManager.Add(entt::hashed_string{"tex_GARAGES"}, "asset/textures/GARAGES.png");
    textureManager.Add(entt::hashed_string{"tex_GBLADESD"}, "asset/textures/GBLADESD.png");
    textureManager.Add(entt::hashed_string{"tex_GBLADESE"}, "asset/textures/GBLADESE.png");
    textureManager.Add(entt::hashed_string{"tex_GCAB"}, "asset/textures/GCAB.png");
    textureManager.Add(entt::hashed_string{"tex_GDOOR"}, "asset/textures/GDOOR.png");
    textureManager.Add(entt::hashed_string{"tex_GRASSA"}, "asset/textures/GRASSA.png");
    textureManager.Add(entt::hashed_string{"tex_GRASSAX"}, "asset/textures/GRASSAX.png");
    textureManager.Add(entt::hashed_string{"tex_GRASSB"}, "asset/textures/GRASSB.png");
    textureManager.Add(entt::hashed_string{"tex_GRASSC"}, "asset/textures/GRASSC.png");
    textureManager.Add(entt::hashed_string{"tex_GRASSRUF"}, "asset/textures/GRASSRUF.png");
    textureManager.Add(entt::hashed_string{"tex_GRASSYARD"}, "asset/textures/GRASSYARD.png");
    textureManager.Add(entt::hashed_string{"tex_GRAVTRANS"}, "asset/textures/GRAVTRANS.png");
    textureManager.Add(entt::hashed_string{"tex_GRDRA"}, "asset/textures/GRDRA.png");
    textureManager.Add(entt::hashed_string{"tex_GRDRB"}, "asset/textures/GRDRB.png");
    textureManager.Add(entt::hashed_string{"tex_GRLT"}, "asset/textures/GRLT.png");
    textureManager.Add(entt::hashed_string{"tex_GRNGLOW"}, "asset/textures/GRNGLOW.png");
    textureManager.Add(entt::hashed_string{"tex_GRVL"}, "asset/textures/GRVL.png");
    textureManager.Add(entt::hashed_string{"tex_GSSIDE"}, "asset/textures/GSSIDE.png");
    textureManager.Add(entt::hashed_string{"tex_GURDRA"}, "asset/textures/GURDRA.png");
    textureManager.Add(entt::hashed_string{"tex_GURDRB"}, "asset/textures/GURDRB.png");
    textureManager.Add(entt::hashed_string{"tex_GURDRC"}, "asset/textures/GURDRC.png");
    textureManager.Add(entt::hashed_string{"tex_GURDRCRSSA"}, "asset/textures/GURDRCRSSA.png");
    textureManager.Add(entt::hashed_string{"tex_HILLSIDE"}, "asset/textures/HILLSIDE.png");
    textureManager.Add(entt::hashed_string{"tex_INBUILDA"}, "asset/textures/INBUILDA.png");
    textureManager.Add(entt::hashed_string{"tex_INBUILDB"}, "asset/textures/INBUILDB.png");
    textureManager.Add(entt::hashed_string{"tex_LIGHTA"}, "asset/textures/LIGHTA.png");
    textureManager.Add(entt::hashed_string{"tex_LOGOA"}, "asset/textures/LOGOA.png");
    textureManager.Add(entt::hashed_string{"tex_LOGOB"}, "asset/textures/LOGOB.png");
    textureManager.Add(entt::hashed_string{"tex_LOGOC"}, "asset/textures/LOGOC.png");
    textureManager.Add(entt::hashed_string{"tex_LOGOD"}, "asset/textures/LOGOD.png");
    textureManager.Add(entt::hashed_string{"tex_LOGOE"}, "asset/textures/LOGOE.png");
    textureManager.Add(entt::hashed_string{"tex_LOGOSYN"}, "asset/textures/LOGOSYN.png");
    textureManager.Add(entt::hashed_string{"tex_OCLOGO"}, "asset/textures/OCLOGO.png");
    textureManager.Add(entt::hashed_string{"tex_OUTHOUSE"}, "asset/textures/OUTHOUSE.png");
    textureManager.Add(entt::hashed_string{"tex_OVRHDLGHT"}, "asset/textures/OVRHDLGHT.png");
    textureManager.Add(entt::hashed_string{"tex_PBOXA"}, "asset/textures/PBOXA.png");
    textureManager.Add(entt::hashed_string{"tex_PEOPLEA"}, "asset/textures/PEOPLEA.png");
    textureManager.Add(entt::hashed_string{"tex_PEOPLEB"}, "asset/textures/PEOPLEB.png");
    textureManager.Add(entt::hashed_string{"tex_PEOPLSDA"}, "asset/textures/PEOPLSDA.png");
    textureManager.Add(entt::hashed_string{"tex_PEOPLSDB"}, "asset/textures/PEOPLSDB.png");
    textureManager.Add(entt::hashed_string{"tex_PEOPLSDC"}, "asset/textures/PEOPLSDC.png");
    textureManager.Add(entt::hashed_string{"tex_PIPES"}, "asset/textures/PIPES.png");
    textureManager.Add(entt::hashed_string{"tex_RAILA"}, "asset/textures/RAILA.png");
    textureManager.Add(entt::hashed_string{"tex_RDGLOW"}, "asset/textures/RDGLOW.png");
    textureManager.Add(entt::hashed_string{"tex_RDLT"}, "asset/textures/RDLT.png");
    textureManager.Add(entt::hashed_string{"tex_RGEDGE"}, "asset/textures/RGEDGE.png");
    textureManager.Add(entt::hashed_string{"tex_RMBLA"}, "asset/textures/RMBLA.png");
    textureManager.Add(entt::hashed_string{"tex_RMBLB"}, "asset/textures/RMBLB.png");
    textureManager.Add(entt::hashed_string{"tex_ROADA"}, "asset/textures/ROADA.png");
    textureManager.Add(entt::hashed_string{"tex_ROADAB"}, "asset/textures/ROADAB.png");
    textureManager.Add(entt::hashed_string{"tex_ROADB"}, "asset/textures/ROADB.png");
    textureManager.Add(entt::hashed_string{"tex_ROADGFLOOR"}, "asset/textures/ROADGFLOOR.png");
    textureManager.Add(entt::hashed_string{"tex_ROADGRID"}, "asset/textures/ROADGRID.png");
    textureManager.Add(entt::hashed_string{"tex_ROADPIT"}, "asset/textures/ROADPIT.png");
    textureManager.Add(entt::hashed_string{"tex_ROADSTRP"}, "asset/textures/ROADSTRP.png");
    textureManager.Add(entt::hashed_string{"tex_RVA"}, "asset/textures/RVA.png");
    textureManager.Add(entt::hashed_string{"tex_SIGN_100"}, "asset/textures/SIGN_100.png");
    textureManager.Add(entt::hashed_string{"tex_SIGN_150"}, "asset/textures/SIGN_150.png");
    textureManager.Add(entt::hashed_string{"tex_SIGN_50"}, "asset/textures/SIGN_50.png");
    textureManager.Add(entt::hashed_string{"tex_STAIRSA"}, "asset/textures/STAIRSA.png");
    textureManager.Add(entt::hashed_string{"tex_STARTLINE"}, "asset/textures/STARTLINE.png");
    textureManager.Add(entt::hashed_string{"tex_TBOOTH"}, "asset/textures/TBOOTH.png");
    textureManager.Add(entt::hashed_string{"tex_TBRANCHA"}, "asset/textures/TBRANCHA.png");
    textureManager.Add(entt::hashed_string{"tex_TCAN"}, "asset/textures/TCAN.png");
    textureManager.Add(entt::hashed_string{"tex_TENTA"}, "asset/textures/TENTA.png");
    textureManager.Add(entt::hashed_string{"tex_TENTB"}, "asset/textures/TENTB.png");
    textureManager.Add(entt::hashed_string{"tex_TENTC"}, "asset/textures/TENTC.png");
    textureManager.Add(entt::hashed_string{"tex_TENTD"}, "asset/textures/TENTD.png");
    textureManager.Add(entt::hashed_string{"tex_TIRESA"}, "asset/textures/TIRESA.png");
    textureManager.Add(entt::hashed_string{"tex_TREELRGA"}, "asset/textures/TREELRGA.png");
    textureManager.Add(entt::hashed_string{"tex_TREELRGB"}, "asset/textures/TREELRGB.png");
    textureManager.Add(entt::hashed_string{"tex_TREELRGC"}, "asset/textures/TREELRGC.png");
    textureManager.Add(entt::hashed_string{"tex_TREEWALLA"}, "asset/textures/TREEWALLA.png");
    textureManager.Add(entt::hashed_string{"tex_TRETRNKA"}, "asset/textures/TRETRNKA.png");
    textureManager.Add(entt::hashed_string{"tex_UNDERA"}, "asset/textures/UNDERA.png");
    textureManager.Add(entt::hashed_string{"tex_WALLGA"}, "asset/textures/WALLGA.png");
    textureManager.Add(entt::hashed_string{"tex_WALL_BAIG"}, "asset/textures/WALL_BAIG.png");
    textureManager.Add(entt::hashed_string{"tex_WALL_BLUEA"}, "asset/textures/WALL_BLUEA.png");
    textureManager.Add(entt::hashed_string{"tex_WALL_WHTA"}, "asset/textures/WALL_WHTA.png");
    textureManager.Add(entt::hashed_string{"tex_WDFENCE"}, "asset/textures/WDFENCE.png");
    textureManager.Add(entt::hashed_string{"tex_WINDA"}, "asset/textures/WINDA.png");
    textureManager.Add(entt::hashed_string{"tex_WIREA"}, "asset/textures/WIREA.png");
    textureManager.Add(entt::hashed_string{"tex_YLOGLOW"}, "asset/textures/YLOGLOW.png");
    textureManager.Add(entt::hashed_string{"tex_YLOLT"}, "asset/textures/YLOLT.png");
}
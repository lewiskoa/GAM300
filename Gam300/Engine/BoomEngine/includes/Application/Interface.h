/**
 * @file    Interface.h
 * @brief   Core interface for engine layers and layer-management utilities.
 */

#pragma once
#ifndef INTERFACE_H
#define INTERFACE_H

#include "Context.h"
#include "Auxiliaries/DataSerializer.h"

namespace Boom
{

    /**
     * @struct   AppInterface
     * @brief    Base interface every engine layer derives from.
     *
     * Provides lifecycle hooks (OnStart, OnUpdate) and
     * layer-management utilities via templates.
     */
    struct BOOM_API AppInterface
    {
        /**
         * @brief  Virtual destructor for safe polymorphic cleanup.
         *
         * BOOM_INLINE hints the compiler to inline this call,
         * minimizing overhead in the engine�s hot loops.
         */
        BOOM_INLINE virtual ~AppInterface() = default;

        /**
         * @brief   Retrieve the first attached layer of type Layer.
         * @tparam  Layer  Must derive from AppInterface.
         * @return  Pointer to the layer instance, or nullptr if not found.
         */
        template<typename Layer>
        BOOM_INLINE Layer* GetLayer()
        {
            // Compile-time check: Layer inherits from AppInterface
            BOOM_STATIC_ASSERT(std::is_base_of<AppInterface, Layer>::value);

            // Search the context�s layer list by matching the LayerID
            auto it = std::find_if(
                m_Context->layers.begin(),
                m_Context->layers.end(),
                [](AppInterface* layer)
                {
                    return layer->m_LayerID == TypeID<Layer>();
                });

            return (it != m_Context->layers.end())
                ? static_cast<Layer*>(*it)
                : nullptr;
        }

        /**
         * @brief   Create and attach a new layer of type Layer.
         * @tparam  Layer      Must derive from AppInterface.
         * @param   args       Arguments forwarded to Layer�s constructor.
         * @return  Pointer to the newly created layer, or nullptr on error.
         */
        template<typename Layer, typename... Args>
        BOOM_INLINE Layer* AttachLayer(Args&&... args)
        {
            BOOM_STATIC_ASSERT(std::is_base_of<AppInterface, Layer>::value);

            if (GetLayer<Layer>() != nullptr)
            {
                BOOM_ERROR("Layer already attached!");
                return nullptr;
            }

            auto* layer = new Layer(std::forward<Args>(args)...);
            m_Context->layers.push_back(layer);

            layer->_InitLayer(m_Context, TypeID<Layer>());

         
            AppInterface* asBase = layer;
            asBase->OnStart();
            return layer;
        }
        BOOM_INLINE double GetDeltaTime() const noexcept { return m_Context->DeltaTime; }

        BOOM_INLINE std::shared_ptr<GLFWwindow> GetWindowHandle()
        {
            return m_Context->window->Handle();
        }

        

    /////////////////////////////////////
    //EventSystem Manipulation Logistics
    /////////////////////////////////////
    public:
        // attach event callback
        template <typename Event, typename Callback>
        BOOM_INLINE void AttachCallback(Callback&& callback)
        {
            m_Context->dispatcher.AttachCallback<Event>
                (std::move(callback), m_LayerID);
        }

        // post event
        template <typename Event, typename... Args>
        BOOM_INLINE void PostEvent(Args&&...args)
        {
            m_Context->dispatcher.PostEvent<Event>(std::forward<Args>
                (args)...);
        }

        // post task event
        template <typename Task>
        BOOM_INLINE void PostTask(Task&& task)
        {
            m_Context->dispatcher.PostTask(std::move(task));
        }

        // detach callback
        template <typename Event>
        BOOM_INLINE void DetachCallback()
        {
            m_Context->dispatcher.DetachCallback<Event>(m_LayerID);
        }

        

    /////////////////////////////////////
    //Entity Manipulation Logistics
    /////////////////////////////////////
    public: 
        // create entity
        template <typename Entt, typename... Args>
        BOOM_INLINE Entt CreateEntt(Args&&... args)
        {
            BOOM_STATIC_ASSERT(std::is_base_of<Entity, Entt>::value);
            return std::move(Entt(&m_Context->scene, std::forward<Args>(args)...));
        }
        // convert id to entity
        template<typename Entt>
        BOOM_INLINE Entt ToEntt(EntityID entity)
        {
            BOOM_STATIC_ASSERT(std::is_base_of<Entity,
                Entt>::value);
            return std::move(Entt(&m_Context->scene, entity));
        }
        // loop through entities
        template<typename Entt, typename Comp, typename Task>
        BOOM_INLINE void EnttView(Task&& task)
        {
            BOOM_STATIC_ASSERT(std::is_base_of<Entity,
                Entt>::value);
            m_Context->scene.view<Comp>().each([this, &task]
            (auto entity, auto& comp)
                {
                    task(std::move(Entt(&m_Context->scene,
                        entity)), comp);
                });
        }
        BOOM_INLINE EntityRegistry& GetEntityRegistry() {
            return m_Context->scene;
        }
        //if you need to swap selected object call with (true)
        // otherwise if used for comparison/no reset of selected needed
        BOOM_INLINE entt::entity& SelectedEntity(bool isResetAllSelected = false) {
            if (isResetAllSelected) ResetAllSelected();
            return selectedEntity;
        }
        BOOM_INLINE void ResetAllSelected() {
            selectedEntity = entt::null;
            selectedAsset = {};
            //add more if needed
        }
    private: //vars
        entt::entity selectedEntity{ entt::null };
        
    /////////////////////////////////////
    //Asset Manipulation Logistics
    /////////////////////////////////////
    public:
        struct AssetInfo {
            AssetID id{};
            AssetType type{};
            std::string name{};
        };

        template<class Type>
        BOOM_INLINE std::string_view GetAssetName(AssetID uid) {
            Type* asset{ m_Context->assets->TryGet<Type>(uid) };
            if (!asset) return "";
            return asset->name;
        }

        template<typename Task>
        BOOM_INLINE void AssetView(Task&& task) {
            m_Context->assets->View([&](auto asset) { task(asset); });
        }

        //Task should be of type [&](TextureAsset* )
        template<typename Task>
        BOOM_INLINE void AssetTextureView(Task&& task) {
            auto& map = m_Context->assets->GetMap<TextureAsset>();
            for (auto& [uid, asset] : map) {
                if (uid != EMPTY_ASSET) {
                    TextureAsset* tex{ dynamic_cast<TextureAsset*>(asset.get()) };
                    if (!tex) continue;
                    task(tex);
                }
            }
        }

     


        BOOM_INLINE uint32_t GetSceneFrame()
        {
			return m_Context->renderer->GetFrame();
        }

        BOOM_INLINE AppContext* GetContext() const noexcept { return m_Context; }
        //if you need to swap selected object call with (true)
        // otherwise if used for comparison/no reset of selected needed
        BOOM_INLINE AssetInfo& SelectedAsset(bool isResetAllSelected = false) {
            if (isResetAllSelected) ResetAllSelected();
            return selectedAsset;
        }

        template<typename Task>
        BOOM_INLINE void ModifyAsset(Task f) {
            switch (selectedAsset.type) {
            case AssetType::TEXTURE:
                m_Context->assets->ModifyTextureFromID(selectedAsset.id, f);
                break;
            case AssetType::MATERIAL:
                m_Context->assets->ModifyMaterialFromID(selectedAsset.id, f);
                break;
            default:
                break;
            }

        }

        BOOM_INLINE AssetRegistry& GetAssetRegistry() {
            return *m_Context->assets;
        }

        BOOM_INLINE void DeleteAsset(AssetID uid, AssetType type) {
            if (type == AssetType::TEXTURE) m_Context->assets->Remove<TextureAsset>(uid);
            else if (type == AssetType::MATERIAL) m_Context->assets->Remove<MaterialAsset>(uid);
            else if (type == AssetType::SKYBOX) m_Context->assets->Remove<SkyboxAsset>(uid);
            else if (type == AssetType::SCRIPT) m_Context->assets->Remove<ScriptAsset>(uid);
            else if (type == AssetType::SCENE) m_Context->assets->Remove<SceneAsset>(uid);
            else if (type == AssetType::MODEL) m_Context->assets->Remove<ModelAsset>(uid);
            else if (type == AssetType::PREFAB) m_Context->assets->Remove<PrefabAsset>(uid);
        }

        BOOM_INLINE bool SaveAssets(const std::string& scenePath = "AssetsProp/")
        {
            //Try blocks cause crashed in release mode. Need to find new alternative
            DataSerializer serializer;

            const std::string assetsFilePath = scenePath + "assets.yaml";

            BOOM_INFO("[Assets] Saving assets to '{}'", assetsFilePath);

            // Serialize scene and assets
            serializer.Serialize(GetAssetRegistry(), assetsFilePath);

            BOOM_INFO("[Assets] Successfully saved assets");
            return true;
        }

        //Gets asset if from path for 100% unique ids
        BOOM_INLINE AssetID AssetIDFromPath(const std::filesystem::path& path)
        {
            return std::hash<std::string>{}(path.string());
        }
    private: //vars
        AssetInfo selectedAsset{};
    
    /////////////////////////////////////
    //virtual
    /////////////////////////////////////
    protected:
        /** @brief  Called once when the layer is attached. Override to initialize. */
        BOOM_INLINE virtual void OnStart() {}

        /** @brief  Called each frame. Override for per-frame logic. */
        BOOM_INLINE virtual void OnUpdate() {}

        AppContext* m_Context{};   ///< Pointer to shared application context
        void _InitLayer(AppContext* ctx, uint32_t id) {
            m_Context = ctx;
            m_LayerID = id;
        }
    private:
        // Allow Application to set up layers
        friend struct Application;

        uint32_t    m_LayerID{};   ///< Unique identifier for this layer
    };

} // namespace Boom

#endif // INTERFACE_H

#include "Core.h"
#include "Auxiliaries/SerializationRegistry.h"

namespace Boom
{
    // Combined initialization function
    void InitializeSerializationSystem()
    {
        BOOM_INFO("[SerializationSystem] Initializing...");

        RegisterAllAssetSerializers();
        RegisterAllComponentSerializers();

        BOOM_INFO("[SerializationSystem] Initialization complete");
    }

    // Auto-initialization class
    // This will be instantiated as a static variable, ensuring registration happens on DLL load
    class SerializationSystemAutoInit
    {
    public:
        SerializationSystemAutoInit()
        {
            InitializeSerializationSystem();
        }

        ~SerializationSystemAutoInit()
        {
            BOOM_INFO("[SerializationSystem] Shutting down");
        }
    };

    // Static instance - will be constructed when the DLL loads
    // This ensures all serializers are registered before any user code runs
    static SerializationSystemAutoInit g_serializationAutoInit;
}
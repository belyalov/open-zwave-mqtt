#include <openzwave/Manager.h>

namespace OpenZWave
{
    namespace Internal
    {
        namespace Platform
        {
            Mutex::Mutex()
            {
            }

            Mutex::~Mutex()
            {
            }
            
            bool Mutex::IsSignalled()
            {
                return false;
            }

            Wait::Wait()
            {
            }

            Wait::~Wait()
            {
            }
        }
    }
}

#ifndef ADSBDECODER_MR_IMPL_BASE_H
#define ADSBDECODER_MR_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Resource_impl.h>
#include <ossie/ThreadedComponent.h>

#include <bulkio/bulkio.h>

class ADSBdecoder_MR_base : public Resource_impl, protected ThreadedComponent
{
    public:
        ADSBdecoder_MR_base(const char *uuid, const char *label);
        ~ADSBdecoder_MR_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        short DebugMode;
        short InteractiveMode;
        short StatsMode;
        double userLat;
        double userLong;
        short NetMode;
        std::string NetOnlyMode;
        short ModeAC;
        short AggressiveMode;
        CORBA::LongLong MessageCount;
        CORBA::ULongLong PreambleCount;

        // Ports
        bulkio::InOctetPort *input;

    private:
};
#endif // ADSBDECODER_MR_IMPL_BASE_H

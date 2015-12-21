#include "ADSBdecoder_MR_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

ADSBdecoder_MR_base::ADSBdecoder_MR_base(const char *uuid, const char *label) :
    Resource_impl(uuid, label),
    ThreadedComponent()
{
    loadProperties();

    input = new bulkio::InOctetPort("input");
    addPort("input", input);
}

ADSBdecoder_MR_base::~ADSBdecoder_MR_base()
{
    delete input;
    input = 0;
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void ADSBdecoder_MR_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    Resource_impl::start();
    ThreadedComponent::startThread();
}

void ADSBdecoder_MR_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    Resource_impl::stop();
    if (!ThreadedComponent::stopThread()) {
        throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }
}

void ADSBdecoder_MR_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    Resource_impl::releaseObject();
}

void ADSBdecoder_MR_base::loadProperties()
{
    addProperty(DebugMode,
                0,
                "DebugMode",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(InteractiveMode,
                1,
                "InteractiveMode",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(StatsMode,
                0,
                "StatsMode",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(userLat,
                39,
                "userLat",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(userLong,
                80,
                "userLong",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(NetMode,
                1,
                "NetMode",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(NetOnlyMode,
                "0",
                "NetOnlyMode",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(ModeAC,
                1,
                "ModeAC",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(AggressiveMode,
                1,
                "AggressiveMode",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(MessageCount,
                "MessageCount",
                "",
                "readonly",
                "",
                "external",
                "configure");

    addProperty(PreambleCount,
                0LL,
                "PreambleCount",
                "",
                "readwrite",
                "",
                "external",
                "configure");

}



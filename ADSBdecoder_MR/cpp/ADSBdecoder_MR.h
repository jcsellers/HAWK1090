/*"This software was developed at SPAWAR Systems Center Atlantic by employees of the Federal Government in the course of their official duties. Pursuant to title 17 Section 105 of the United States Code this software is not subject to copyright protection and is in the public domain. The Government assumes no responsibility whatsoever for its use by other parties, and the software is provided "AS IS" without warranty or guarantee of any kind, express or implied about its quality, reliability, or any other characteristic. In no event shall the Government be liable for any claim, damages or other liability, whether in an action of contract, tort or other dealings in the software.   The Government has no obligation hereunder to provide maintenance, support, updates, enhancements, or modifications.  We would appreciate acknowledgement if the software is used. This software can be redistributed and/or modified freely provided that any derivative works bear some notice that they are derived from it, and any modified versions bear some notice that they have been modified." 
*/

#ifndef ADSBDECODER_MR_IMPL_H
#define ADSBDECODER_MR_IMPL_H

#include "ADSBdecoder_MR_base.h"


// Include all the functionality from C source files
extern "C"
{
#include "dump1090.h"
}

class ADSBdecoder_MR_i : public ADSBdecoder_MR_base
{
    ENABLE_LOGGING
    uint32_t buf_size;

    public:
        ADSBdecoder_MR_i(const char *uuid, const char *label);
        ~ADSBdecoder_MR_i();
        int serviceFunction();


    private:
        void computeMagnitudeVector(std::vector<uint16_t> p, uint32_t in_buffer_length);
        void setInteractiveMode(const short *oldValue, const short *newValue);


};

#endif // ADSBDECODER_MR_IMPL_H

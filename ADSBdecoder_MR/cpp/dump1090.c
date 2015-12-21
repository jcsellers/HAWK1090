/* 
 * Modified by SPAWAR Atlantic. 
 * This software is provided with BSD rights.
 * Original files retain their original rights/licenses.
 * This is prototype/demo code and is provided with no warranties or guarantees, use at your own risk.
 * Major modifications to this file from it's original version include the removal of unused functions,
 * and the modification of functions within
 *
 */

// dump1090, a Mode S messages decoder for RTLSDR devices.
//
// Copyright (C) 2012 by Salvatore Sanfilippo <antirez@gmail.com>
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//  *  Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//
//  *  Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
#include "coaa.h"
#include "dump1090.h"

//
// =============================== Terminal handling ========================
//
int getTermRows() { return MODES_INTERACTIVE_ROWS;}

//
// =============================== Initialization ===========================
//
void modesInitConfig(void) {
    // Default everything to zero/NULL
    memset(&Modes, 0, sizeof(Modes));

    // Now initialise things that should not be 0/NULL to their defaults
    Modes.check_crc               = 1;
    Modes.net_heartbeat_rate      = MODES_NET_HEARTBEAT_RATE;
    Modes.net_output_sbs_port     = MODES_NET_OUTPUT_SBS_PORT;
    Modes.net_output_raw_port     = MODES_NET_OUTPUT_RAW_PORT;
    Modes.net_input_raw_port      = MODES_NET_INPUT_RAW_PORT;
    Modes.net_output_beast_port   = MODES_NET_OUTPUT_BEAST_PORT;
    Modes.net_input_beast_port    = MODES_NET_INPUT_BEAST_PORT;
    Modes.net_http_port           = MODES_NET_HTTP_PORT;
    Modes.net_fatsv_port          = MODES_NET_OUTPUT_FA_TSV_PORT;
    Modes.interactive_rows        = getTermRows();
    Modes.interactive_delete_ttl  = MODES_INTERACTIVE_DELETE_TTL;
    Modes.interactive_display_ttl = MODES_INTERACTIVE_DISPLAY_TTL;
    Modes.fUserLat                = MODES_USER_LATITUDE_DFLT;
    Modes.fUserLon                = MODES_USER_LONGITUDE_DFLT;
}

void modesInit(void) {
    int i, q;

    // Allocate the various buffers used by Modes
    	// The malloc for Modes.magnitude ought to be sized dynamically based on input buffer size, could cause issue if input buffer
    	// turns out to be bigger than the constant value of MODES_ASYNC_BUF_SIZE, fix later
    if ( ((Modes.icao_cache = (uint32_t *) malloc(sizeof(uint32_t) * MODES_ICAO_CACHE_LEN * 2)                  ) == NULL) ||
         ((Modes.magnitude  = (uint16_t *) malloc(MODES_ASYNC_BUF_SIZE+MODES_PREAMBLE_SIZE+MODES_LONG_MSG_SIZE) ) == NULL) ||
         ((Modes.maglut     = (uint16_t *) malloc(sizeof(uint16_t) * 256 * 256)                                 ) == NULL) ||
         ((Modes.beastOut   = (char     *) malloc(MODES_RAWOUT_BUF_SIZE)                                        ) == NULL) ||
         ((Modes.rawOut     = (char     *) malloc(MODES_RAWOUT_BUF_SIZE)                                        ) == NULL) ) 
    {
        fprintf(stderr, "Out of memory allocating data buffer.\n");
        exit(1);
    }

    // Clear the buffers that have just been allocated, just in-case
    memset(Modes.icao_cache, 0,   sizeof(uint32_t) * MODES_ICAO_CACHE_LEN * 2);
    memset(Modes.magnitude,  0,   MODES_ASYNC_BUF_SIZE+MODES_PREAMBLE_SIZE+MODES_LONG_MSG_SIZE);

    // Validate the users Lat/Lon home location inputs
    if ( (Modes.fUserLat >   90.0)  // Latitude must be -90 to +90
      || (Modes.fUserLat <  -90.0)  // and 
      || (Modes.fUserLon >  360.0)  // Longitude must be -180 to +360
      || (Modes.fUserLon < -180.0) ) {
        Modes.fUserLat = Modes.fUserLon = 0.0;
    } else if (Modes.fUserLon > 180.0) { // If Longitude is +180 to +360, make it -180 to 0
        Modes.fUserLon -= 360.0;
    }
    // If both Lat and Lon are 0.0 then the users location is either invalid/not-set, or (s)he's in the 
    // Atlantic ocean off the west coast of Africa. This is unlikely to be correct. 
    // Set the user LatLon valid flag only if either Lat or Lon are non zero. Note the Greenwich meridian 
    // is at 0.0 Lon,so we must check for either fLat or fLon being non zero not both. 
    // Testing the flag at runtime will be much quicker than ((fLon != 0.0) || (fLat != 0.0))
    Modes.bUserFlags &= ~MODES_USER_LATLON_VALID;
    if ((Modes.fUserLat != 0.0) || (Modes.fUserLon != 0.0)) {
        Modes.bUserFlags |= MODES_USER_LATLON_VALID;
    }

    // Limit the maximum requested raw output size to less than one Ethernet Block 
    if (Modes.net_output_raw_size > (MODES_RAWOUT_BUF_FLUSH))
      {Modes.net_output_raw_size = MODES_RAWOUT_BUF_FLUSH;}
    if (Modes.net_output_raw_rate > (MODES_RAWOUT_BUF_RATE))
      {Modes.net_output_raw_rate = MODES_RAWOUT_BUF_RATE;}
    if (Modes.net_sndbuf_size > (MODES_NET_SNDBUF_MAX))
      {Modes.net_sndbuf_size = MODES_NET_SNDBUF_MAX;}

    // Initialise the Block Timers to something half sensible
    ftime(&Modes.stSystemTimeBlk);
    for (i = 0; i < MODES_ASYNC_BUF_NUMBER; i++)
      {Modes.stSystemTimeRTL[i] = Modes.stSystemTimeBlk;}

    // Each I and Q value varies from 0 to 255, which represents a range from -1 to +1. To get from the 
    // unsigned (0-255) range you therefore subtract 127 (or 128 or 127.5) from each I and Q, giving you 
    // a range from -127 to +128 (or -128 to +127, or -127.5 to +127.5)..
    //
    // To decode the AM signal, you need the magnitude of the waveform, which is given by sqrt((I^2)+(Q^2))
    // The most this could be is if I&Q are both 128 (or 127 or 127.5), so you could end up with a magnitude 
    // of 181.019 (or 179.605, or 180.312)
    //
    // However, in reality the magnitude of the signal should never exceed the range -1 to +1, because the 
    // values are I = rCos(w) and Q = rSin(w). Therefore the integer computed magnitude should (can?) never 
    // exceed 128 (or 127, or 127.5 or whatever)
    //
    // If we scale up the results so that they range from 0 to 65535 (16 bits) then we need to multiply 
    // by 511.99, (or 516.02 or 514). antirez's original code multiplies by 360, presumably because he's 
    // assuming the maximim calculated amplitude is 181.019, and (181.019 * 360) = 65166.
    //
    // So lets see if we can improve things by subtracting 127.5, Well in integer arithmatic we can't
    // subtract half, so, we'll double everything up and subtract one, and then compensate for the doubling 
    // in the multiplier at the end.
    //
    // If we do this we can never have I or Q equal to 0 - they can only be as small as +/- 1.
    // This gives us a minimum magnitude of root 2 (0.707), so the dynamic range becomes (1.414-255). This 
    // also affects our scaling value, which is now 65535/(255 - 1.414), or 258.433254
    //
    // The sums then become mag = 258.433254 * (sqrt((I*2-255)^2 + (Q*2-255)^2) - 1.414)
    //                   or mag = (258.433254 * sqrt((I*2-255)^2 + (Q*2-255)^2)) - 365.4798
    //
    // We also need to clip mag just incaes any rogue I/Q values somehow do have a magnitude greater than 255.
    //

    for (i = 0; i <= 255; i++) {
        for (q = 0; q <= 255; q++) {
            int mag, mag_i, mag_q;

            mag_i = (i * 2) - 255;
            mag_q = (q * 2) - 255;

            mag = (int) round((sqrt((mag_i*mag_i)+(mag_q*mag_q)) * 258.433254) - 365.4798);

            Modes.maglut[(i*256)+q] = (uint16_t) ((mag < 65535) ? mag : 65535);
        }
    }

    // Prepare error correction tables
    modesInitErrorInfo();

}

//
//=========================================================================
//
// This function outputs various statistics to the console window, it is by
// default configured to be called upon termination of the waveform in the
// components destructor.
//
void display_stats(void) {
    int j;
    time_t now = time(NULL);

    printf("\n\n");
    if (Modes.interactive)
        interactiveShowData();

    printf("Statistics as at %s", ctime(&now));

    printf("%d sample blocks processed\n",                    Modes.stat_blocks_processed);
    printf("%d sample blocks dropped\n",                      Modes.stat_blocks_dropped);

    printf("%d ModeA/C detected\n",                           Modes.stat_ModeAC);
    printf("%d valid Mode-S preambles\n",                     Modes.stat_valid_preamble);
    printf("%d DF-?? fields corrected for length\n",          Modes.stat_DF_Len_Corrected);
    printf("%d DF-?? fields corrected for type\n",            Modes.stat_DF_Type_Corrected);
    printf("%d demodulated with 0 errors\n",                  Modes.stat_demodulated0);
    printf("%d demodulated with 1 error\n",                   Modes.stat_demodulated1);
    printf("%d demodulated with 2 errors\n",                  Modes.stat_demodulated2);
    printf("%d demodulated with > 2 errors\n",                Modes.stat_demodulated3);
    printf("%d with good crc\n",                              Modes.stat_goodcrc);
    printf("%d with bad crc\n",                               Modes.stat_badcrc);
    printf("%d errors corrected\n",                           Modes.stat_fixed);

    for (j = 0;  j < MODES_MAX_BITERRORS;  j++) {
        printf("   %d with %d bit %s\n", Modes.stat_bit_fix[j], j+1, (j==0)?"error":"errors");
    }

    if (Modes.phase_enhance) {
        printf("%d phase enhancement attempts\n",                 Modes.stat_out_of_phase);
        printf("%d phase enhanced demodulated with 0 errors\n",   Modes.stat_ph_demodulated0);
        printf("%d phase enhanced demodulated with 1 error\n",    Modes.stat_ph_demodulated1);
        printf("%d phase enhanced demodulated with 2 errors\n",   Modes.stat_ph_demodulated2);
        printf("%d phase enhanced demodulated with > 2 errors\n", Modes.stat_ph_demodulated3);
        printf("%d phase enhanced with good crc\n",               Modes.stat_ph_goodcrc);
        printf("%d phase enhanced with bad crc\n",                Modes.stat_ph_badcrc);
        printf("%d phase enhanced errors corrected\n",            Modes.stat_ph_fixed);

        for (j = 0;  j < MODES_MAX_BITERRORS;  j++) {
            printf("   %d with %d bit %s\n", Modes.stat_ph_bit_fix[j], j+1, (j==0)?"error":"errors");
        }
    }

    printf("%d total usable messages\n",                      Modes.stat_goodcrc + Modes.stat_ph_goodcrc + Modes.stat_fixed + Modes.stat_ph_fixed);
    fflush(stdout);

    Modes.stat_blocks_processed =
        Modes.stat_blocks_dropped = 0;

    Modes.stat_ModeAC =
        Modes.stat_valid_preamble =
        Modes.stat_DF_Len_Corrected =
        Modes.stat_DF_Type_Corrected =
        Modes.stat_demodulated0 =
        Modes.stat_demodulated1 =
        Modes.stat_demodulated2 =
        Modes.stat_demodulated3 =
        Modes.stat_goodcrc =
        Modes.stat_badcrc =
        Modes.stat_fixed = 0;

    Modes.stat_out_of_phase =
        Modes.stat_ph_demodulated0 =
        Modes.stat_ph_demodulated1 =
        Modes.stat_ph_demodulated2 =
        Modes.stat_ph_demodulated3 =
        Modes.stat_ph_goodcrc =
        Modes.stat_ph_badcrc =
        Modes.stat_ph_fixed = 0;

    for (j = 0;  j < MODES_MAX_BITERRORS;  j++) {
        Modes.stat_ph_bit_fix[j] = 0;
        Modes.stat_bit_fix[j] = 0;
    }
}

//
//=========================================================================
//
// This function is called a few times every second by main in order to
// perform tasks we need to do continuously, like accepting new clients
// from the net, refreshing the screen in interactive mode, and so forth
//
void backgroundTasks(void) {
    static time_t next_stats;

    if (Modes.net) {
        modesReadFromClients();
        showFlightsFATSV();
    }    

    // If Modes.aircrafts is not NULL, remove any stale aircraft
    if (Modes.aircrafts) {
        interactiveRemoveStaleAircrafts();
    }

    // Refresh screen when in interactive mode
    if (Modes.interactive) {
        interactiveShowData();
    }

    if (Modes.stats > 0) {
        time_t now = time(NULL);
        if (now > next_stats) {
            if (next_stats != 0)
                //display_stats();
            next_stats = now + Modes.stats;
        }
    }
}

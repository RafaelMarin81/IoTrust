/*
 * Copyright (c) 2018, Department of Information and Communication Engineering.
 * University of Murcia. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author(s):
 *            Jorge Gallego Madrid <jorge.gallego1@um.es>
 *            Jesús Sánchez Gómez  <jesus.sanchez4@um.es>
 */

/**
 * \brief The database of the different rules used by the SCHC C/A
 * procedure.
 */

/**********************************************************************/
/***        Include files                                           ***/
/**********************************************************************/

/**********************************************************************/
/***        Local Include files                                     ***/
/**********************************************************************/

#include "schc.h"
#include "context.h"
#include "ipv6_layer.h"

/**********************************************************************/
/***        Macro Definitions                                       ***/
/**********************************************************************/

/**********************************************************************/
/***        Types Definitions                                       ***/
/**********************************************************************/

/**********************************************************************/
/***        Forward Declarations                                    ***/
/**********************************************************************/

/**********************************************************************/
/***        Constants                                               ***/
/**********************************************************************/

/**********************************************************************/
/***        Global Variables                                        ***/
/**********************************************************************/

/**
 * \brief The SCHC rules of the context used by the schc_compress() and
 * schc_decompress().
 *
 *
 * The schc_compress() and schc_decompress() functions will lookup the
 * information in these rules to determine how to apply
 * compression/decompression to a packet (if it matches).
 *
 * \note Because we use uint8_t type as Rule ID, we can have as many as
 * 255 rules. (Very unlikely for our purposes right now).
 *
 * \note The first rule that matches is the one which will be used to
 * compress. In the draft, the procedure should find the one which
 * matches and also offers the better Compression size, but in our
 * implementation, the first that matches is the one that is used. TODO
 * Think about how to implement this.
 *
 * \note The TV must be a valid string. Also, the format is not checked
 * before parsing, this is extremely dangerous in the case of
 * IPV6_DEV_PREFIX, IPV6_DEVIID, etc. Extreme caution must be taken when
 * writing the rules to make sure that the format is right.
 *
 * TODO It should not use a fixed size of [14], it should be dynamic
 * size of rows. Think about this. The ammount of rules should be
 * computed at runtime in the for() loops.
 */
const struct field_description rules[][33] = {
	
	{ /* Dummy rule 0: fport can not be 0 */ 
		/* Field;              FL; FP;DI; TV;                 MO;     CA; */
		{ IPV6_VERSION,        4,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ IPV6_TRAFFIC_CLASS,  8,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ IPV6_FLOW_LABEL,     20, 1, BI, "0",                IGNORE, NOT_SENT         },
		{ IPV6_PAYLOAD_LENGTH, 16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ IPV6_NEXT_HEADER,    8,  1, BI, "17",               EQUALS, NOT_SENT         },
		{ IPV6_HOP_LIMIT,      8,  1, BI, "64",               IGNORE, NOT_SENT         },
		{ IPV6_DEV_PREFIX,     64, 1, BI, "1234567891234567", EQUALS, NOT_SENT         },
		{ IPV6_DEVIID,         64, 1, BI, "7157084458723854", EQUALS, NOT_SENT         },
		{ IPV6_APP_PREFIX,     64, 1, BI, "1234567890123456", EQUALS, NOT_SENT         },
		{ IPV6_APPIID,         64, 1, BI, "1478585784768976", EQUALS, NOT_SENT         },

		{ UDP_DEVPORT,         16, 1, BI, "0",             	  IGNORE, VALUE_SENT       },
		{ UDP_APPPORT,         16, 1, BI, "5683",             IGNORE, NOT_SENT         },
		{ UDP_LENGTH,          16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ UDP_CHECKSUM,        16, 1, BI, "0",                IGNORE, COMPUTE_CHECKSUM },

		{ COAP_VERSION,        2,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_TYPE,           2,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ COAP_TKL,            4,  1, BI, "2",                EQUALS, NOT_SENT         },
		{ COAP_CODE,           8,  1, BI, "2",                EQUALS, NOT_SENT         },
		{ COAP_MESSAGE_ID,     16, 1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_TOKEN,          16, 1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_DELTA,   8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_VALUE,   16, 1, BI, "0",                IGNORE, VALUE_SENT       },
		{ END_OF_RULE                                                                  },

	},

	{ /* CoAP rule 1: used for the Message 1 of LO-CoAP-EAP Exchange */
		/* Field;              FL; FP;DI; TV;                 MO;     CA; */
		{ IPV6_VERSION,        4,  1, BI, "6",                EQUALS, NOT_SENT         },
		{ IPV6_TRAFFIC_CLASS,  8,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ IPV6_FLOW_LABEL,     20, 1, BI, "0",                IGNORE, NOT_SENT         },
		{ IPV6_PAYLOAD_LENGTH, 16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ IPV6_NEXT_HEADER,    8,  1, BI, "17",               EQUALS, NOT_SENT         },
		{ IPV6_HOP_LIMIT,      8,  1, BI, "64",               IGNORE, NOT_SENT         },
		{ IPV6_DEV_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_DEVIID,         64, 1, BI, "080027fffe000000", EQUALS, NOT_SENT         },
		{ IPV6_APP_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_APPIID,         64, 1, BI, "0A0027FFFE542E4A", EQUALS, NOT_SENT         },

		{ UDP_DEVPORT,         16, 1, BI, "59355",            EQUALS, NOT_SENT         },
		{ UDP_APPPORT,         16, 1, BI, "5683",             EQUALS, NOT_SENT         },
		{ UDP_LENGTH,          16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ UDP_CHECKSUM,        16, 1, BI, "0",                IGNORE, COMPUTE_CHECKSUM },

		{ COAP_VERSION,        2,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_TYPE,           2,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_TKL,            4,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ COAP_CODE,           8,  1, BI, "2",                EQUALS, NOT_SENT         },
		{ COAP_MESSAGE_ID,     16, 1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_TOKEN,          16, 1, BI, "0",                IGNORE, NOT_SENT         },
		{ COAP_OPTION_DELTA,   8,  1, BI, "11",               EQUALS, NOT_SENT         },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_OPTION_VALUE,   16, 1, BI, "b",                EQUALS, NOT_SENT         },
		{ COAP_OPTION_DELTA,   8,  1, BI, "28",               EQUALS, NOT_SENT         },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "4",                EQUALS, NOT_SENT         },
		{ COAP_OPTION_VALUE,   16, 1, BI, "",                 IGNORE, VALUE_SENT       },
		{ COAP_OPTION_DELTA,   8,  1, BI, "29",               EQUALS, NOT_SENT         },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_OPTION_VALUE,   16, 1, BI, "",                 IGNORE, VALUE_SENT       },

      { END_OF_RULE,                                                                 },

	},

	{ /* CoAP rule 2: used for the Message 1 of LO-CoAP-EAP Exchange */ 
		/* Field;              FL; FP;DI; TV;                 MO;     CA; */
		{ IPV6_VERSION,        4,  1, BI, "6",                EQUALS, NOT_SENT         },
		{ IPV6_TRAFFIC_CLASS,  8,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ IPV6_FLOW_LABEL,     20, 1, BI, "0",                IGNORE, NOT_SENT         },
		{ IPV6_PAYLOAD_LENGTH, 16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ IPV6_NEXT_HEADER,    8,  1, BI, "17",               EQUALS, NOT_SENT         },
		{ IPV6_HOP_LIMIT,      8,  1, BI, "64",               IGNORE, NOT_SENT         },
		{ IPV6_DEV_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_DEVIID,         64, 1, BI, "080027fffe000000", EQUALS, NOT_SENT         },
		{ IPV6_APP_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_APPIID,         64, 1, BI, "0A0027FFFE542E4A", EQUALS, NOT_SENT         },

		{ UDP_DEVPORT,         16, 1, BI, "59355",            EQUALS, NOT_SENT         },
		{ UDP_APPPORT,         16, 1, BI, "5683",             EQUALS, NOT_SENT         },
		{ UDP_LENGTH,          16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ UDP_CHECKSUM,        16, 1, BI, "0",                IGNORE, COMPUTE_CHECKSUM },

		{ COAP_VERSION,        2,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_TYPE,           2,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ COAP_TKL,            4,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ COAP_CODE,           8,  1, BI, "2",                EQUALS, NOT_SENT         },
		{ COAP_MESSAGE_ID,     16, 1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_TOKEN,          16, 1, BI, "0",                IGNORE, NOT_SENT         },
		{ COAP_OPTION_DELTA,   8,  1, BI, "11",               EQUALS, NOT_SENT         },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_OPTION_VALUE,   16, 1, BI, "b",                EQUALS, NOT_SENT         },

      { END_OF_RULE,                                                                 },

	},

	{ /* CoAP rule 3: used for the Message 1 of LO-CoAP-EAP Exchange */ 
		/* Field;              FL; FP;DI; TV;                 MO;     CA; */
		{ IPV6_VERSION,        4,  1, BI, "6",                EQUALS, NOT_SENT         },
		{ IPV6_TRAFFIC_CLASS,  8,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ IPV6_FLOW_LABEL,     20, 1, BI, "0",                IGNORE, NOT_SENT         },
		{ IPV6_PAYLOAD_LENGTH, 16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ IPV6_NEXT_HEADER,    8,  1, BI, "17",               EQUALS, NOT_SENT         },
		{ IPV6_HOP_LIMIT,      8,  1, BI, "64",               IGNORE, NOT_SENT         },
		{ IPV6_DEV_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_DEVIID,         64, 1, BI, "080027fffe000000", EQUALS, NOT_SENT         },
		{ IPV6_APP_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_APPIID,         64, 1, BI, "0A0027FFFE542E4A", EQUALS, NOT_SENT         },

		{ UDP_DEVPORT,         16, 1, BI, "59355",            EQUALS, NOT_SENT         },
		{ UDP_APPPORT,         16, 1, BI, "5683",             EQUALS, NOT_SENT         },
		{ UDP_LENGTH,          16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ UDP_CHECKSUM,        16, 1, BI, "0",                IGNORE, COMPUTE_CHECKSUM },

		{ COAP_VERSION,        2,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_TYPE,           2,  1, BI, "2",                EQUALS, NOT_SENT         },
		{ COAP_TKL,            4,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ COAP_CODE,           8,  1, BI, "65",               EQUALS, NOT_SENT         },
		{ COAP_MESSAGE_ID,     16, 1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_TOKEN,          16, 1, BI, "0",                IGNORE, NOT_SENT         },
		{ COAP_OPTION_DELTA,   8,  1, BI, "11",               EQUALS, NOT_SENT         },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_OPTION_VALUE,   16, 1, BI, "b",                EQUALS, NOT_SENT         },
		{ COAP_OPTION_DELTA,   8,  1, BI, "11",               EQUALS, NOT_SENT         },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_OPTION_VALUE,   16, 1, BI, "",                 IGNORE, VALUE_SENT       },

      { END_OF_RULE,                                                                 },

	},

	{ /* CoAP rule 4: used for the Message 1 of LO-CoAP-EAP Exchange */ 
		/* Field;              FL; FP;DI; TV;                 MO;     CA; */
		{ IPV6_VERSION,        4,  1, BI, "6",                EQUALS, NOT_SENT         },
		{ IPV6_TRAFFIC_CLASS,  8,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ IPV6_FLOW_LABEL,     20, 1, BI, "0",                IGNORE, NOT_SENT         },
		{ IPV6_PAYLOAD_LENGTH, 16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ IPV6_NEXT_HEADER,    8,  1, BI, "17",               EQUALS, NOT_SENT         },
		{ IPV6_HOP_LIMIT,      8,  1, BI, "64",               IGNORE, NOT_SENT         },
		{ IPV6_DEV_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_DEVIID,         64, 1, BI, "080027fffe000000", EQUALS, NOT_SENT         },
		{ IPV6_APP_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_APPIID,         64, 1, BI, "0A0027FFFE542E4A", EQUALS, NOT_SENT         },

		{ UDP_DEVPORT,         16, 1, BI, "59355",            EQUALS, NOT_SENT         },
		{ UDP_APPPORT,         16, 1, BI, "5683",             EQUALS, NOT_SENT         },
		{ UDP_LENGTH,          16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ UDP_CHECKSUM,        16, 1, BI, "0",                IGNORE, COMPUTE_CHECKSUM },

		{ COAP_VERSION,        2,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_TYPE,           2,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ COAP_TKL,            4,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ COAP_CODE,           8,  1, BI, "2",                EQUALS, NOT_SENT         },
		{ COAP_MESSAGE_ID,     16, 1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_TOKEN,          16, 1, BI, "0",                IGNORE, NOT_SENT         },
		{ COAP_OPTION_DELTA,   8,  1, BI, "11",               EQUALS, NOT_SENT         },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_OPTION_VALUE,   16, 1, BI, "b",                EQUALS, NOT_SENT         },
		{ COAP_OPTION_DELTA,   8,  1, BI, "11",               EQUALS, NOT_SENT         },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_OPTION_VALUE,   16, 1, BI, "",                 IGNORE, VALUE_SENT       },

      { END_OF_RULE,                                                                 },

	},


	{ /* CoAP rule 5: used for the Message 1 of LO-CoAP-EAP Exchange */ 
		/* Field;              FL; FP;DI; TV;                 MO;     CA; */
		{ IPV6_VERSION,        4,  1, BI, "6",                EQUALS, NOT_SENT         },
		{ IPV6_TRAFFIC_CLASS,  8,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ IPV6_FLOW_LABEL,     20, 1, BI, "0",                IGNORE, NOT_SENT         },
		{ IPV6_PAYLOAD_LENGTH, 16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ IPV6_NEXT_HEADER,    8,  1, BI, "17",               EQUALS, NOT_SENT         },
		{ IPV6_HOP_LIMIT,      8,  1, BI, "64",               IGNORE, NOT_SENT         },
		{ IPV6_DEV_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_DEVIID,         64, 1, BI, "080027fffe000000", EQUALS, NOT_SENT         },
		{ IPV6_APP_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_APPIID,         64, 1, BI, "0A0027FFFE542E4A", EQUALS, NOT_SENT         },

		{ UDP_DEVPORT,         16, 1, BI, "59355",            EQUALS, NOT_SENT         },
		{ UDP_APPPORT,         16, 1, BI, "5683",             EQUALS, NOT_SENT         },
		{ UDP_LENGTH,          16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ UDP_CHECKSUM,        16, 1, BI, "0",                IGNORE, COMPUTE_CHECKSUM },

		{ COAP_VERSION,        2,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_TYPE,           2,  1, BI, "2",                EQUALS, NOT_SENT         },
		{ COAP_TKL,            4,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ COAP_CODE,           8,  1, BI, "68",               EQUALS, NOT_SENT         },
		{ COAP_MESSAGE_ID,     16, 1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_TOKEN,          16, 1, BI, "0",                IGNORE, NOT_SENT         },

      { END_OF_RULE,                                                                 },

	},


	{ /* CoAP rule 6: used for the Message 1 of LO-CoAP-EAP Exchange */ 
		/* Field;              FL; FP;DI; TV;                 MO;     CA; */
		{ IPV6_VERSION,        4,  1, BI, "6",                EQUALS, NOT_SENT         },
		{ IPV6_TRAFFIC_CLASS,  8,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ IPV6_FLOW_LABEL,     20, 1, BI, "0",                IGNORE, NOT_SENT         },
		{ IPV6_PAYLOAD_LENGTH, 16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ IPV6_NEXT_HEADER,    8,  1, BI, "17",               EQUALS, NOT_SENT         },
		{ IPV6_HOP_LIMIT,      8,  1, BI, "64",               IGNORE, NOT_SENT         },
		{ IPV6_DEV_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_DEVIID,         64, 1, BI, "080027fffe000000", EQUALS, NOT_SENT         },
		{ IPV6_APP_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_APPIID,         64, 1, BI, "0A0027FFFE542E4A", EQUALS, NOT_SENT         },

		{ UDP_DEVPORT,         16, 1, BI, "59355",            EQUALS, NOT_SENT         },
		{ UDP_APPPORT,         16, 1, BI, "5683",             EQUALS, NOT_SENT         },
		{ UDP_LENGTH,          16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ UDP_CHECKSUM,        16, 1, BI, "0",                IGNORE, COMPUTE_CHECKSUM },

		{ COAP_VERSION,        2,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_TYPE,           2,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ COAP_TKL,            4,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ COAP_CODE,           8,  1, BI, "2",                EQUALS, NOT_SENT         },
		{ COAP_MESSAGE_ID,     16, 1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_TOKEN,          16, 1, BI, "0",                IGNORE, NOT_SENT         },
		{ COAP_OPTION_DELTA,   8,  1, BI, "11",               EQUALS, NOT_SENT         },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_OPTION_VALUE,   16, 1, BI, "b",                EQUALS, NOT_SENT         },
		{ COAP_OPTION_DELTA,   8,  1, BI, "11",               EQUALS, NOT_SENT         },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_OPTION_VALUE,   16, 1, BI, "",                 IGNORE, VALUE_SENT       },
		{ COAP_OPTION_DELTA,   8,  1, BI, "28",               EQUALS, NOT_SENT         },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "4",                EQUALS, NOT_SENT         },
		{ COAP_OPTION_VALUE,   16, 1, BI, "",                 IGNORE, VALUE_SENT       },
		{ COAP_OPTION_DELTA,   8,  1, BI, "92",               EQUALS, NOT_SENT         },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "16",               EQUALS, NOT_SENT         },
		{ COAP_OPTION_VALUE,   16, 1, BI, "",                 IGNORE, VALUE_SENT       },

      { END_OF_RULE,                                                                 },

	},

	{ /* CoAP rule 7: used for the Message 1 of LO-CoAP-EAP Exchange */ 
		/* Field;              FL; FP;DI; TV;                 MO;     CA; */
		{ IPV6_VERSION,        4,  1, BI, "6",                EQUALS, NOT_SENT         },
		{ IPV6_TRAFFIC_CLASS,  8,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ IPV6_FLOW_LABEL,     20, 1, BI, "0",                IGNORE, NOT_SENT         },
		{ IPV6_PAYLOAD_LENGTH, 16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ IPV6_NEXT_HEADER,    8,  1, BI, "17",               EQUALS, NOT_SENT         },
		{ IPV6_HOP_LIMIT,      8,  1, BI, "64",               IGNORE, NOT_SENT         },
		{ IPV6_DEV_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_DEVIID,         64, 1, BI, "080027fffe000000", EQUALS, NOT_SENT         },
		{ IPV6_APP_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_APPIID,         64, 1, BI, "0A0027FFFE542E4A", EQUALS, NOT_SENT         },

		{ UDP_DEVPORT,         16, 1, BI, "59355",            EQUALS, NOT_SENT         },
		{ UDP_APPPORT,         16, 1, BI, "5683",             EQUALS, NOT_SENT         },
		{ UDP_LENGTH,          16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ UDP_CHECKSUM,        16, 1, BI, "0",                IGNORE, COMPUTE_CHECKSUM },

		{ COAP_VERSION,        2,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_TYPE,           2,  1, BI, "2",                EQUALS, NOT_SENT         },
		{ COAP_TKL,            4,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ COAP_CODE,           8,  1, BI, "68",               EQUALS, NOT_SENT         },
		{ COAP_MESSAGE_ID,     16, 1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_TOKEN,          16, 1, BI, "0",                IGNORE, NOT_SENT         },
		{ COAP_OPTION_DELTA,   8,  1, BI, "92",               EQUALS, NOT_SENT         },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "16",               EQUALS, NOT_SENT         },
		{ COAP_OPTION_VALUE,   16, 1, BI, "",                 IGNORE, VALUE_SENT       },

      { END_OF_RULE,                                                                 },

	},

	{ /* SCHC rule 8: used for testing purposes of fragmentation with dummy CoAP payloads
        One CoAP Option (whatever it contains) */ 
		/* Field;              FL; FP;DI; TV;                 MO;     CA; */
		{ IPV6_VERSION,        4,  1, BI, "6",                EQUALS, NOT_SENT         },
		{ IPV6_TRAFFIC_CLASS,  8,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ IPV6_FLOW_LABEL,     20, 1, BI, "0",                IGNORE, NOT_SENT         },
		{ IPV6_PAYLOAD_LENGTH, 16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ IPV6_NEXT_HEADER,    8,  1, BI, "17",               EQUALS, NOT_SENT         },
		{ IPV6_HOP_LIMIT,      8,  1, BI, "64",               IGNORE, NOT_SENT         },
		{ IPV6_DEV_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_DEVIID,         64, 1, BI, "080027fffe000000", EQUALS, NOT_SENT         },
		{ IPV6_APP_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_APPIID,         64, 1, BI, "0A0027FFFE542E4A", EQUALS, NOT_SENT         },

		{ UDP_DEVPORT,         16, 1, BI, "59355",            EQUALS, NOT_SENT         },
		{ UDP_APPPORT,         16, 1, BI, "5683",             EQUALS, NOT_SENT         },
		{ UDP_LENGTH,          16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ UDP_CHECKSUM,        16, 1, BI, "0",                IGNORE, COMPUTE_CHECKSUM },

		{ COAP_VERSION,        2,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_TYPE,           2,  1, BI, "2",                EQUALS, NOT_SENT         },
		{ COAP_TKL,            4,  1, BI, "2",                EQUALS, NOT_SENT         },
		{ COAP_CODE,           8,  1, BI, "65",               EQUALS, NOT_SENT         },
		{ COAP_MESSAGE_ID,     16, 1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_TOKEN,          16, 1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_DELTA,   8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_VALUE,   16, 1, BI, "",                 IGNORE, VALUE_SENT       },

      { END_OF_RULE,                                                                 },
	},


	{ /* SCHC rule 9: used for testing purposes of fragmentation with dummy CoAP payloads
        Two CoAP Option (whatever it contains) */ 
		/* Field;              FL; FP;DI; TV;                 MO;     CA; */
		{ IPV6_VERSION,        4,  1, BI, "6",                EQUALS, NOT_SENT         },
		{ IPV6_TRAFFIC_CLASS,  8,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ IPV6_FLOW_LABEL,     20, 1, BI, "0",                IGNORE, NOT_SENT         },
		{ IPV6_PAYLOAD_LENGTH, 16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ IPV6_NEXT_HEADER,    8,  1, BI, "17",               EQUALS, NOT_SENT         },
		{ IPV6_HOP_LIMIT,      8,  1, BI, "64",               IGNORE, NOT_SENT         },
		{ IPV6_DEV_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_DEVIID,         64, 1, BI, "080027fffe000000", EQUALS, NOT_SENT         },
		{ IPV6_APP_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_APPIID,         64, 1, BI, "0A0027FFFE542E4A", EQUALS, NOT_SENT         },

		{ UDP_DEVPORT,         16, 1, BI, "59355",            EQUALS, NOT_SENT         },
		{ UDP_APPPORT,         16, 1, BI, "5683",             EQUALS, NOT_SENT         },
		{ UDP_LENGTH,          16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ UDP_CHECKSUM,        16, 1, BI, "0",                IGNORE, COMPUTE_CHECKSUM },

		{ COAP_VERSION,        2,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_TYPE,           2,  1, BI, "2",                EQUALS, NOT_SENT         },
		{ COAP_TKL,            4,  1, BI, "2",                EQUALS, NOT_SENT         },
		{ COAP_CODE,           8,  1, BI, "65",               EQUALS, NOT_SENT         },
		{ COAP_MESSAGE_ID,     16, 1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_TOKEN,          16, 1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_DELTA,   8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_VALUE,   16, 1, BI, "",                 IGNORE, VALUE_SENT       },
		{ COAP_OPTION_DELTA,   8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_VALUE,   16, 1, BI, "",                 IGNORE, VALUE_SENT       },

      { END_OF_RULE,                                                                 },
	},


	{ /* SCHC rule 10: used for testing purposes of fragmentation with dummy CoAP payloads
        Three CoAP Option (whatever it contains) */ 
		/* Field;              FL; FP;DI; TV;                 MO;     CA; */
		{ IPV6_VERSION,        4,  1, BI, "6",                EQUALS, NOT_SENT         },
		{ IPV6_TRAFFIC_CLASS,  8,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ IPV6_FLOW_LABEL,     20, 1, BI, "0",                IGNORE, NOT_SENT         },
		{ IPV6_PAYLOAD_LENGTH, 16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ IPV6_NEXT_HEADER,    8,  1, BI, "17",               EQUALS, NOT_SENT         },
		{ IPV6_HOP_LIMIT,      8,  1, BI, "64",               IGNORE, NOT_SENT         },
		{ IPV6_DEV_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_DEVIID,         64, 1, BI, "080027fffe000000", EQUALS, NOT_SENT         },
		{ IPV6_APP_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_APPIID,         64, 1, BI, "0A0027FFFE542E4A", EQUALS, NOT_SENT         },

		{ UDP_DEVPORT,         16, 1, BI, "59355",            EQUALS, NOT_SENT         },
		{ UDP_APPPORT,         16, 1, BI, "5683",             EQUALS, NOT_SENT         },
		{ UDP_LENGTH,          16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ UDP_CHECKSUM,        16, 1, BI, "0",                IGNORE, COMPUTE_CHECKSUM },

		{ COAP_VERSION,        2,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_TYPE,           2,  1, BI, "2",                EQUALS, NOT_SENT         },
		{ COAP_TKL,            4,  1, BI, "2",                EQUALS, NOT_SENT         },
		{ COAP_CODE,           8,  1, BI, "65",               EQUALS, NOT_SENT         },
		{ COAP_MESSAGE_ID,     16, 1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_TOKEN,          16, 1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_DELTA,   8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_VALUE,   16, 1, BI, "",                 IGNORE, VALUE_SENT       },
		{ COAP_OPTION_DELTA,   8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_VALUE,   16, 1, BI, "",                 IGNORE, VALUE_SENT       },
		{ COAP_OPTION_DELTA,   8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_VALUE,   16, 1, BI, "",                 IGNORE, VALUE_SENT       },

      { END_OF_RULE,                                                                 },
	},


	{ /* SCHC rule 11: used for testing purposes of fragmentation with dummy CoAP payloads
        Four CoAP Option (whatever it contains) */ 
		/* Field;              FL; FP;DI; TV;                 MO;     CA; */
		{ IPV6_VERSION,        4,  1, BI, "6",                EQUALS, NOT_SENT         },
		{ IPV6_TRAFFIC_CLASS,  8,  1, BI, "0",                EQUALS, NOT_SENT         },
		{ IPV6_FLOW_LABEL,     20, 1, BI, "0",                IGNORE, NOT_SENT         },
		{ IPV6_PAYLOAD_LENGTH, 16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ IPV6_NEXT_HEADER,    8,  1, BI, "17",               EQUALS, NOT_SENT         },
		{ IPV6_HOP_LIMIT,      8,  1, BI, "64",               IGNORE, NOT_SENT         },
		{ IPV6_DEV_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_DEVIID,         64, 1, BI, "080027fffe000000", EQUALS, NOT_SENT         },
		{ IPV6_APP_PREFIX,     64, 1, BI, "FE80000000000000", EQUALS, NOT_SENT         },
		{ IPV6_APPIID,         64, 1, BI, "0A0027FFFE542E4A", EQUALS, NOT_SENT         },

		{ UDP_DEVPORT,         16, 1, BI, "59355",            EQUALS, NOT_SENT         },
		{ UDP_APPPORT,         16, 1, BI, "5683",             EQUALS, NOT_SENT         },
		{ UDP_LENGTH,          16, 1, BI, "0",                IGNORE, COMPUTE_LENGTH   },
		{ UDP_CHECKSUM,        16, 1, BI, "0",                IGNORE, COMPUTE_CHECKSUM },

		{ COAP_VERSION,        2,  1, BI, "1",                EQUALS, NOT_SENT         },
		{ COAP_TYPE,           2,  1, BI, "2",                EQUALS, NOT_SENT         },
		{ COAP_TKL,            4,  1, BI, "2",                EQUALS, NOT_SENT         },
		{ COAP_CODE,           8,  1, BI, "65",               EQUALS, NOT_SENT         },
		{ COAP_MESSAGE_ID,     16, 1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_TOKEN,          16, 1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_DELTA,   8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_VALUE,   16, 1, BI, "",                 IGNORE, VALUE_SENT       },
		{ COAP_OPTION_DELTA,   8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_VALUE,   16, 1, BI, "",                 IGNORE, VALUE_SENT       },
		{ COAP_OPTION_DELTA,   8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_VALUE,   16, 1, BI, "",                 IGNORE, VALUE_SENT       },
		{ COAP_OPTION_DELTA,   8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_LENGTH,  8,  1, BI, "0",                IGNORE, VALUE_SENT       },
		{ COAP_OPTION_VALUE,   16, 1, BI, "",                 IGNORE, VALUE_SENT       },

      { END_OF_RULE,                                                                 },
	},


};

/**********************************************************************/
/***        AUX Functions                                           ***/
/**********************************************************************/

/**********************************************************************/
/***        MAIN INO routines                                       ***/
/**********************************************************************/

/**********************************************************************/
/***        END OF FILE                                             ***/
/**********************************************************************/

/*
 * Deprecated or testing functions start here.
 */

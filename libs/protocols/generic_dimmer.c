/*
	Copyright (C) 2013 CurlyMo

	This file is part of pilight.

    pilight is free software: you can redistribute it and/or modify it under the 
	terms of the GNU General Public License as published by the Free Software 
	Foundation, either version 3 of the License, or (at your option) any later 
	version.

    pilight is distributed in the hope that it will be useful, but WITHOUT ANY 
	WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR 
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with pilight. If not, see	<http://www.gnu.org/licenses/>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../../pilight.h"
#include "common.h"
#include "log.h"
#include "protocol.h"
#include "hardware.h"
#include "binary.h"
#include "gc.h"
#include "generic_dimmer.h"

void genDimCreateMessage(int id, int state, int dimlevel) {
	generic_dimmer->message = json_mkobject();
	json_append_member(generic_dimmer->message, "id", json_mknumber(id));
	if(dimlevel >= 0) {
		state = 1;
		json_append_member(generic_dimmer->message, "dimlevel", json_mknumber(dimlevel));
	}
	if(state == 1) {
		json_append_member(generic_dimmer->message, "state", json_mkstring("on"));
	} else {
		json_append_member(generic_dimmer->message, "state", json_mkstring("off"));
	}
}

int genDimcheckValues(JsonNode *code) {
	int dimlevel = -1;
	int max = 15;
	int min = 0;
	double itmp = -1;
	
	if(json_find_number(code, "dimlevel-maximum", &itmp) == 0)
		max = (int)round(itmp);
	if(json_find_number(code, "dimlevel-minimum", &itmp) == 0)
		min = (int)round(itmp);	
	if(json_find_number(code, "dimlevel", &itmp) == 0)
		dimlevel = (int)round(itmp);

	if(min > max) {
		return 1;
	}
	
	if(dimlevel != -1) {
		if(dimlevel < min || dimlevel > max) {
			return 1;
		} else {
			return 0;
		}
	}
	return 0;
}

int genDimCreateCode(JsonNode *code) {
	int id = -1;
	int state = -1;
	int dimlevel = -1;
	int max = 0;
	int min = 10;
	double itmp = -1;

	if(json_find_number(code, "dimlevel-maximum", &itmp) == 0)
		max = (int)round(itmp);
	if(json_find_number(code, "dimlevel-minimum", &itmp) == 0)
		min = (int)round(itmp);

	if(json_find_number(code, "id", &itmp) == 0)
		id = (int)round(itmp);
	if(json_find_number(code, "dimlevel", &itmp) == 0)
		dimlevel = (int)round(itmp);
	if(json_find_number(code, "off", &itmp) == 0)
		state=0;
	else if(json_find_number(code, "on", &itmp) == 0)
		state=1;

	if(id == -1 || (dimlevel == -1 && state == -1)) {
		logprintf(LOG_ERR, "generic_dimmer: insufficient number of arguments");
		return EXIT_FAILURE;
	} else if(dimlevel != -1 && (dimlevel > max || dimlevel < min)) {
		logprintf(LOG_ERR, "arctech_dimmer: invalid dimlevel range");
		return EXIT_FAILURE;		
	} else if(dimlevel >= 0 && state == 0) {
		logprintf(LOG_ERR, "generic_dimmer: dimlevel and state cannot be combined");
		return EXIT_FAILURE;
	} else {
		if(dimlevel >= 0) {
			state = -1;
		}
		genDimCreateMessage(id, state, dimlevel);
	}
	return EXIT_SUCCESS;

}

void genDimPrintHelp(void) {
	printf("\t -t --on\t\t\tsend an on signal\n");
	printf("\t -f --off\t\t\tsend an off signal\n");
	printf("\t -i --id=id\t\t\tcontrol a device with this id\n");
	printf("\t -d --dimlevel=dimlevel\t\tsend a specific dimlevel\n");	
}

void genDimInit(void) {
	
	protocol_register(&generic_dimmer);
	protocol_set_id(generic_dimmer, "generic_dimmer");
	protocol_device_add(generic_dimmer, "generic_dimmer", "Generic Dimmers");
	generic_dimmer->devtype = DIMMER;

	options_add(&generic_dimmer->options, 'd', "dimlevel", OPTION_HAS_VALUE, CONFIG_VALUE, JSON_NUMBER, NULL, "^([0-9]{1,})$");
	options_add(&generic_dimmer->options, 't', "on", OPTION_NO_VALUE, CONFIG_STATE, JSON_STRING, NULL, NULL);
	options_add(&generic_dimmer->options, 'f', "off", OPTION_NO_VALUE, CONFIG_STATE, JSON_STRING, NULL, NULL);
	options_add(&generic_dimmer->options, 'i', "id", OPTION_HAS_VALUE, CONFIG_ID, JSON_NUMBER, NULL, "^([0-9]{1,})$");

	options_add(&generic_dimmer->options, 0, "dimlevel-minimum", OPTION_HAS_VALUE, CONFIG_SETTING, JSON_NUMBER, (void *)0, "^([0-9]{1}|[1][0-5])$");
	options_add(&generic_dimmer->options, 0, "dimlevel-maximum", OPTION_HAS_VALUE, CONFIG_SETTING, JSON_NUMBER, (void *)15, "^([0-9]{1}|[1][0-5])$");
	options_add(&generic_dimmer->options, 0, "gui-readonly", OPTION_HAS_VALUE, CONFIG_SETTING, JSON_NUMBER, (void *)0, "^[10]{1}$");

	generic_dimmer->printHelp=&genDimPrintHelp;
	generic_dimmer->createCode=&genDimCreateCode;
	generic_dimmer->checkValues=&genDimcheckValues;
}

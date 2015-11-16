/*
 * ModuloInfo.h
 *
 * Created: 11/5/2015 3:41:38 PM
 *  Author: ekt
 */ 


#ifndef MODULOINFO_H_
#define MODULOINFO_H_

#include <inttypes.h>

#define MODULO_TYPE_SIZE 32

struct ModuloInfo {
	uint16_t id;
	uint16_t version;
	uint8_t type[MODULO_TYPE_SIZE];
};

#define DECLARE_MODULO(moduloType, moduloVersion) \
	ModuloInfo moduloInfo = {.id=0xFFFF, .version=moduloVersion, moduloType};

void LoadModuloInfo();
void SaveModuloInfo();

#endif /* MODULOINFO_H_ */
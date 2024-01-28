#include <nds.h>

#ifndef _NNS_FND_H
#define _NNS_FND_H

#ifdef __cplusplus
extern "C"
{
#endif

	bool NNS_FndMountArchive(void* archive, const char* arcName, void* arcBinary);
			
#ifdef __cplusplus
}
#endif

#endif
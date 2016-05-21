#ifndef __XML_H_
#define __XML_H_

#include "../common/kernel_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

char * XML_GetNodeText(const char *xml_part, const char * nodename, char * output, int output_size);
int LoadXmlParameters(ReducedCosAppXmlInfo * xmlInfo, const char *rpx_name, const char *path);
int GetId6FromMeta(const char *path, char *output);

#ifdef __cplusplus
}
#endif

#endif // __XML_H_

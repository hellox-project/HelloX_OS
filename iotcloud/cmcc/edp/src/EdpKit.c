//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
//#include <assert.h>
//#include <time.h>

#include "../KAPI/K_SOCKET.H"
#include "../KAPI/K_KERNEL.H"
#include "../KAPI/K_IO.H"

#include "EdpKit.h"
 
char *  strncpy (
	char * dest,
	const char * source,
	size_t count
	)
{
	char *start = dest;

	while (count && (*dest++ = *source++))    /* copy string */
		count--;

	if (count)                              /* pad out with zeroes */
		while (--count)
			*dest++ = '\0';

	return(start);
}


//It can handle the scenario that the dst and src memory overlaped scenario.
void* mymemmove(void *dst,const void *src,int n)
{
	char *dp = (char *)dst;
	char *sp = (char *)src; 

	if((NULL == dst) || (NULL == src) || (n <= 0))
	{
		return NULL;
	}

	//Not overlaped.
	if(sp>dp||(sp+n)<dp)
	{ 
		while(n--) 
			*(dp++) = *(sp++);
		*dp = '\0';
	}
	else if(sp<dp) //Overlaped.
	{
		sp += n; 
		dp += n; 
		*dp = '\0'; 
		while(n--)
			*(--dp) = *(--sp); 
	}
	return dst;
}

static void FormatAt(char* buffer, int len, time_t now)
{
	//struct tm timinfo;

	mystrcpy(buffer,"0.0.0:0.0.0");
    //if (now == 0)
	//{
		//now = time(0);
    //}
    
/*#if defined(linux) || defined(__linux) || defined(__linux__)
    strftime(buffer, len, "%F %T", localtime_r(&now, &timinfo));

#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
	
	localtime_s(&timinfo, &now);
    
	strftime(buffer, len, "%F %T", &timinfo); 
#endif*/

}


#define MKFUN_PACKET_SAVE_DATA(TYPE, NAME)				\
    EdpPacket* PacketSavedata##NAME(SaveDataType type,			\
				    const char* dst_devid,		\
				    const char* ds_id,			\
				    TYPE value,				\
				    time_t at,				\
				    const char* token){			\
	EdpPacket* pkg = NULL;						\
    switch (type){							\
    case kTypeFullJson:							\
	pkg = PacketSavedataType1##NAME(dst_devid, ds_id,		\
				  value, at, token);			\
	break;								\
    case kTypeSimpleJsonWithoutTime:					\
	pkg = PacketSavedataType2##NAME(dst_devid, ds_id,		\
				  value, at, token);			\
	break;								\
    case kTypeSimpleJsonWithTime:					\
	pkg = PacketSavedataType3##NAME(dst_devid, ds_id,		\
				  value, at, token);			\
	break;								\
    default:								\
	break;								\
    }									\
									\
    return pkg;								\
}

#define MKFUN_PACKET_SAVE_DATA_TYPE1(TYPE, NAME, MED)			\
    static EdpPacket* PacketSavedataType1##NAME(const char* dst_devid,	\
						const char* ds_id,	\
						TYPE value,		\
						time_t at,		\
						const char* token)	\
{									\
	cJSON* json_obj  ; 				\
	cJSON* ds_array ;                              \
	cJSON* json_obj_ds;								\
	cJSON* dp_array  ;                              \
    cJSON* dp_item   ;                              \
	char   ss[100];						            \
	EdpPacket* pkg;                                  \
	                                                \
	json_obj	= cJSON_CreateObject();             \
	if(token)										\
	{															\
	    cJSON_AddStringToObject(json_obj, "token", token);		\
	}															\
																\
	ds_array = cJSON_CreateArray();								\
	json_obj_ds = cJSON_CreateObject();							\
																\
	cJSON_AddStringToObject(json_obj_ds, "id", ds_id);		    \
									                           \
	dp_array = cJSON_CreateArray();				              \
	dp_item  = cJSON_CreateObject();				          \
														      \
	FormatAt(ss, 100, at);				                    \
	cJSON_AddStringToObject(dp_item, "at", ss);			     \
	cJSON_Add##MED##ToObject(dp_item, "value", value);		\
	cJSON_AddItemToArray(dp_array, dp_item);			\
									\
	cJSON_AddItemToObject(json_obj_ds, "datapoints", dp_array);	\
	cJSON_AddItemToArray(ds_array, json_obj_ds);			\
	cJSON_AddItemToObject(json_obj, "datastreams", ds_array);	\
									                           \
	pkg = PacketSavedataJson(dst_devid, json_obj, kTypeFullJson);		\
	cJSON_Delete(json_obj);						\
									\
	return pkg;							\
}

#define MKFUN_PACKET_SAVE_DATA_TYPE2(TYPE, NAME, MED)			\
    static EdpPacket* PacketSavedataType2##NAME(const char* dst_devid,	\
						const char* ds_id,	\
						TYPE value,		\
						time_t at,		\
						const char* token)	\
    {									\
	cJSON* json_obj = cJSON_CreateObject();				\
	EdpPacket* pkg;                                      \
	cJSON_Add##MED##ToObject(json_obj, ds_id, value);		\
									\
	pkg = PacketSavedataJson(dst_devid, json_obj,	\
					    kTypeSimpleJsonWithoutTime); \
	cJSON_Delete(json_obj);						\
									\
	return pkg;							\
}


#define MKFUN_PACKET_SAVE_DATA_TYPE3(TYPE, NAME, MED)			\
    static EdpPacket* PacketSavedataType3##NAME(const char* dst_devid,	\
						const char* ds_id,	\
						TYPE value,		\
						time_t at,		\
						const char* token)	\
{									\
	cJSON* json_obj  = cJSON_CreateObject();				\
	cJSON* json_obj_ds;								\
	char ss[100];							\
	EdpPacket* pkg;                          \
	FormatAt(ss, 100, at);						\
	json_obj_ds = cJSON_CreateObject();			\
	cJSON_Add##MED##ToObject(json_obj_ds, ss, value);		\
	cJSON_AddItemToObject(json_obj, ds_id, json_obj_ds);		\
									\
	pkg = PacketSavedataJson(dst_devid, json_obj,	\
					    kTypeSimpleJsonWithTime);	\
	cJSON_Delete(json_obj);						\
									\
	return pkg;							\
}

#define MKFUN_UNPACK_SAVE_DATA(TYPE, NAME)				\
    int32 UnpackSavedata##NAME(SaveDataType type,			\
				 EdpPacket* pkg,			\
				 char** ds_id,				\
				 TYPE* value)				\
    {									\
    int ret = -1;							\
    switch (type){							\
    case kTypeFullJson:							\
	ret = UnpackSavedataType1##NAME(pkg, ds_id, value);		\
	break;								\
    case kTypeSimpleJsonWithoutTime:					\
	ret = UnpackSavedataType2##NAME(pkg, ds_id, value);		\
	break;								\
    case kTypeSimpleJsonWithTime:					\
	ret = UnpackSavedataType3##NAME(pkg, ds_id, value);		\
	break;								\
    default:								\
	break;								\
    }									\
									\
    return ret;								\
}

#define MKFUN_UNPACK_SAVE_DATA_TYPE1(TYPE, NAME, MED)			\
    static int32 UnpackSavedataType1##NAME(EdpPacket* pkg,		\
					     char** ds_id,		\
					     TYPE* value)		\
    {									\
    cJSON* json_obj;						\
    cJSON* ds_array;						\
	cJSON* dp_array;						\
	char* id;                              \
	cJSON* ds_item;                         \
	cJSON* dp_item;                         \
	char* valuestring;                       \
											  \
    if(0 != UnpackSavedataJson(pkg, &json_obj))				\
	return ERR_UNPACK_SAVED_JSON;					         \
									                         \
    ds_array = cJSON_GetObjectItem(json_obj, "datastreams"); \
																\
	if((0 == ds_array) || (1 != cJSON_GetArraySize(ds_array)))		\
	{                                                                 \
            return ERR_UNPACK_SAVED_JSON;			 \
	}                                                \
									                  \
    ds_item = cJSON_GetArrayItem(ds_array, 0);			\
													  \
	if(0 == ds_item)							       \
	{return ERR_UNPACK_SAVED_JSON;					}   \
	                                                           \
    id = cJSON_GetObjectItem(ds_item, "id")->valuestring;		\
    *ds_id = (char*)hx_malloc(mystrlen(id)+1);				\
    hx_memcpy((char*)(*ds_id), id, mystrlen(id)+1);				\
    									\
    dp_array = cJSON_GetObjectItem(ds_item, "datapoints");	\
	                                                            \
    if((0 == dp_array) || (1 != cJSON_GetArraySize(dp_array)))		\
	{                                                               \
		return ERR_UNPACK_SAVED_JSON;					            \
	}                                                              \
									                               \
    dp_item = cJSON_GetArrayItem(dp_array, 0);			         \


#define UNPACK_SAVE_DATA_TYPE1_END_STRING				\
    valuestring = cJSON_GetObjectItem(dp_item, "value")->valuestring; \
    *value = (char*)hx_malloc(mystrlen(valuestring)+1);			\
    hx_memcpy((char*)(*value), valuestring, mystrlen(valuestring)+1);	\
    cJSON_Delete(json_obj);						\
    return 0;								\
}

#define UNPACK_SAVE_DATA_TYPE1_END_NUMBER(TYPE)			\
    *value = cJSON_GetObjectItem(dp_item, "value")->value##TYPE;	\
    cJSON_Delete(json_obj);						\
    return 0;								\
}

#define MKFUN_UNPACK_SAVE_DATA_TYPE2(TYPE, NAME, MED)			\
    static int32 UnpackSavedataType2##NAME(EdpPacket* pkg,		\
						  char** ds_id,		\
						  TYPE* value)		\
    {									\
     cJSON* json_obj;				     \
	 cJSON* json_child;                    \
	 char* valuestring;                      \
	  size_t len;                            \
	                                              \
    if(0 != UnpackSavedataJson(pkg, &json_obj))		\
	{ return ERR_UNPACK_SAVED_JSON;		}		\
	                                            \
	if (!json_obj->child)                    \
	{						                   \
	    cJSON_Delete(json_obj);					\
	    return ERR_UNPACK_SAVED_JSON;			\
	}								            \
	                                             \
	json_child = json_obj->child;				\
	len = mystrlen(json_child->string) + 1;			\
	*ds_id = (char*)hx_malloc(len);					\
	hx_memcpy((char*)(*ds_id), json_child->string, len);		\

#define UNPACK_SAVE_DATA_TYPE23_END_NUMBER(TYPE)			\
    *value = json_child->value##TYPE;					\
    cJSON_Delete(json_obj);						\
    return 0;								\
}                                             

#define UNPACK_SAVE_DATA_TYPE23_END_STRING()				\
    valuestring = json_child->valuestring;			    \
    *value = (char*)hx_malloc(mystrlen(valuestring)+1);			\
	hx_memcpy((char*)(*value), valuestring, mystrlen(valuestring)+1);	 \
    cJSON_Delete(json_obj);						                 \
    return 0;								\
}

#define MKFUN_UNPACK_SAVE_DATA_TYPE3(TYPE, NAME, MED)			\
    static int32 UnpackSavedataType3##NAME(EdpPacket* pkg,		\
						  char** ds_id,		\
						  TYPE* value)		\
{									\
     cJSON* json_obj;						\
	cJSON* json_child;						\
	char* valuestring;                      \
	size_t len;                               \
	                                          \
   if(0 != UnpackSavedataJson(pkg, &json_obj))			\
	    return ERR_UNPACK_SAVED_JSON;				\
									\
	json_child = json_obj->child;					\
	if (!json_child || !json_child->string){			\
	    cJSON_Delete(json_obj);					\
	    return ERR_UNPACK_SAVED_JSON;				\
	}								\
									\
	len = mystrlen(json_child->string) + 1;			\
	*ds_id = (char*)hx_malloc(len);					\
	hx_memcpy(*ds_id, json_child->string, len);			\
									\
	json_child = json_child->child;					\
	if (!json_child){						\
	    cJSON_Delete(json_obj);					\
	    return ERR_UNPACK_SAVED_JSON;				\
	}								\


/*---------------------------------------------------------------------------*/
Buffer* NewBuffer()
{
    Buffer* buf = (Buffer*)hx_malloc(sizeof(Buffer));

    buf->_data = (uint8*)hx_malloc(sizeof(uint8) * BUFFER_SIZE);
    buf->_write_pos = 0;
    buf->_read_pos = 0;
    buf->_capacity = BUFFER_SIZE;

    return buf;
}
void DeleteBuffer(Buffer** buf)
{
    uint8* pdata = (*buf)->_data;
	
	if(pdata)    hx_free(pdata);

    hx_free(*buf);
    *buf = 0;
}
int32 CheckCapacity(Buffer* buf, uint32 len)
{
    uint32 cap_len = buf->_capacity;
    int32 flag = 0;
    while (cap_len - buf->_write_pos < len) /* remain len < len */
    {
        cap_len = cap_len << 1;
        if (++flag > 32)
            break;  /* overflow */
    }
    if (flag > 32)
        return -1;
    if (cap_len > buf->_capacity)
    {
        uint8* pdata = (uint8*)hx_malloc(sizeof(uint8) * cap_len);
        hx_memcpy(pdata, buf->_data, buf->_write_pos);
        hx_free(buf->_data);
        buf->_data = pdata;
        buf->_capacity = cap_len;
    }
	
    return 0;
}
/*---------------------------------------------------------------------------*/
int32 ReadByte(EdpPacket* pkg, uint8* val)
{
    if (pkg->_read_pos+1 > pkg->_write_pos) 
        return -1;
    *val = pkg->_data[pkg->_read_pos];
    pkg->_read_pos += 1;
    return 0;
}
int32 ReadBytes(EdpPacket* pkg, uint8** val, uint32 count)
{
    if (pkg->_read_pos+count > pkg->_write_pos) 
        return -1;
    *val = (uint8*)hx_malloc(sizeof(uint8) * count);
    hx_memcpy(*val, pkg->_data + pkg->_read_pos, count);
    pkg->_read_pos += count;
    return 0;
}
int32 ReadUint16(EdpPacket* pkg, uint16* val)
{
    uint8 msb, lsb;
    if (pkg->_read_pos+2 > pkg->_write_pos) 
        return -1;
    msb = pkg->_data[pkg->_read_pos];
    pkg->_read_pos++;
    lsb = pkg->_data[pkg->_read_pos];
    pkg->_read_pos++;
    *val = (msb<<8) + lsb;
    return 0;
}
int32 ReadUint32(EdpPacket* pkg, uint32* val)
{
    int32 i = 0;
    uint32 tmpval = 0;
    if (pkg->_read_pos+4 > pkg->_write_pos) 
        return -1;
    while (i++ < 4) {
        tmpval = (tmpval << 8) | (pkg->_data[pkg->_read_pos]);
        pkg->_read_pos++;
    }
    *val = tmpval;
    return 0;
}
int32 ReadStr(EdpPacket* pkg, char** val)
{
    uint16 len;
    /* read str len */
    int rc = ReadUint16(pkg, &len);
    if (rc) 
        return rc;
    if (pkg->_read_pos+len > pkg->_write_pos) 
        return -1;
    /* copy str val */
    *val = (char*)hx_malloc(sizeof(char) * (len + 1));
    mymemset(*val, 0, len+1);
    strncpy(*val, (const char *)(pkg->_data + pkg->_read_pos), len);
    pkg->_read_pos += len;
    return 0;
}
int32 ReadRemainlen(EdpPacket* pkg, uint32* len_val)
{
    uint32 multiplier = 1;
    uint32 len_len = 0;
    uint8 onebyte = 0;
    int32 rc;
    *len_val = 0;
    do {
        rc = ReadByte(pkg, &onebyte);
        if (rc) 
            return rc;

        *len_val += (onebyte & 0x7f) * multiplier;
        multiplier *= 0x80;

        len_len++;
        if (len_len > 4) {
            return -1;//len of len more than 4;
        }
    } while((onebyte & 0x80) != 0);
    return 0;
}
/*---------------------------------------------------------------------------*/
int32 WriteByte(Buffer* buf, uint8 byte)
{
    //assert(buf->_read_pos == 0);
	  if (CheckCapacity(buf, 1))
        return -1;
    buf->_data[buf->_write_pos] = byte;
    buf->_write_pos++;
    return 0;
}
int32 WriteBytes(Buffer* buf, const void* bytes, uint32 count)
{
    //assert(buf->_read_pos == 0);

    if (CheckCapacity(buf, count))
        return -1;
    hx_memcpy(buf->_data + buf->_write_pos, bytes, count);
    buf->_write_pos += count;
    return 0;
}
int32 WriteUint16(Buffer* buf, uint16 val)
{
    //assert(buf->_read_pos == 0);
    return WriteByte(buf, MOSQ_MSB(val)) 
        || WriteByte(buf, MOSQ_LSB(val));
}
int32 WriteUint32(Buffer* buf, uint32 val)
{
   // assert(buf->_read_pos == 0);
    return WriteByte(buf, (val >> 24) & 0x00FF) 
        || WriteByte(buf, (val >> 16) & 0x00FF)
        || WriteByte(buf, (val >> 8) & 0x00FF) 
        || WriteByte(buf, (val) & 0x00FF);
}
int32 WriteStr(Buffer* buf, const char *str)
{
    
    uint16 length = mystrlen(str);

	//assert(buf->_read_pos == 0);

    return WriteUint16(buf, length)        || WriteBytes(buf, str, length);
}
int32 WriteRemainlen(Buffer* buf, uint32 len_val)
{
   // assert(buf->_read_pos == 0);
    uint32 remaining_length = len_val;
    int32 remaining_count = 0;
    uint8 byte = 0;
    do {
        byte = remaining_length % 128;
        remaining_length = remaining_length / 128;
        /* If there are more digits to encode, set the top bit of this digit */
        if (remaining_length > 0) {
            byte = byte | 0x80;
        }
        buf->_data[buf->_write_pos++] = byte;
        remaining_count++;
    }while(remaining_length > 0 && remaining_count < 5);
  //  assert(remaining_count != 5);
    return 0;
}
/*---------------------------------------------------------------------------*/
/* connect1 (C->S): devid + apikey */
EdpPacket* PacketConnect1(const char* devid, const char* auth_key)
{
    EdpPacket* pkg = NewBuffer();
    uint32 remainlen;
	/* msg type */
    WriteByte(pkg, CONNREQ);
	/* remain len */
	remainlen = (2+3)+1+1+2+(2+mystrlen(devid))+(2+mystrlen(auth_key));
	WriteRemainlen(pkg, remainlen);
	/* protocol desc */
	WriteStr(pkg, PROTOCOL_NAME);
	/* protocol version */
	WriteByte(pkg, PROTOCOL_VERSION);
	/* connect flag */
	WriteByte(pkg, 0x40);
	/* keep time */
	WriteUint16(pkg, 0x0080);
	/* DEVID */
	WriteStr(pkg, devid);
	/* auth key */
	WriteStr(pkg, auth_key);
    return pkg;
}
/* connect2 (C->S): userid + auth_info */
EdpPacket* PacketConnect2(const char* userid, const char* auth_info)
{
    EdpPacket* pkg = NewBuffer();
    uint32 remainlen;
	/* msg type */
    WriteByte(pkg, CONNREQ);
	/* remain len */
	remainlen = (2+3)+1+1+2+2+(2+mystrlen(userid))+(2+mystrlen(auth_info));
	WriteRemainlen(pkg, remainlen);
	/* protocol desc */
	WriteStr(pkg, PROTOCOL_NAME);
	/* protocol version */
	WriteByte(pkg, PROTOCOL_VERSION);
	/* connect flag */
	WriteByte(pkg, 0xC0);
	/* keep time */
	WriteUint16(pkg, 0x0080);
    /* devid */
    WriteByte(pkg, 0x00);
    WriteByte(pkg, 0x00);
	/* USERID */
	WriteStr(pkg, userid);
	/* auth info */
	WriteStr(pkg, auth_info);
    return pkg;
}
/* push_data (C->S) */
EdpPacket* PacketPushdata(const char* dst_devid, const char* data, uint32 data_len)
{
    EdpPacket* pkg = NewBuffer();
    uint32 remainlen;
	/* msg type */
	WriteByte(pkg, PUSHDATA);
	/* remain len */
	remainlen = (2+mystrlen(dst_devid))+data_len;
	WriteRemainlen(pkg, remainlen);
	/* dst devid */
	WriteStr(pkg, dst_devid);
	/* data */
	WriteBytes(pkg, data, data_len);
    return pkg;
}

/*void printjson(cJSON* json_obj)
{
	if(!json_obj) return;

	_hx_printf("json type=%d,c=0x%X,p=0x%X n=0x%X",
		json_obj->type,json_obj->child,json_obj->prev,json_obj->next);
	
	printjson(json_obj->child);
	printjson(json_obj->next);
	
}*/
/* sava_data (C->S) */
EdpPacket* PacketSavedataJson(const char* dst_devid, cJSON* json_obj, int type)
{
    EdpPacket* pkg    = NewBuffer();
    uint32 remainlen  = 0;
    char* json_out    = cJSON_Print(json_obj);
    uint32 json_len   = mystrlen(json_out);

	//printjson(json_obj);//_hx_printf("json type=%d,c=0x%X,p=0x%X n=0x%X",json_obj->type,json_obj->child,json_obj->prev,json_obj->next);
	//return 0;

	/* msg type */
	WriteByte(pkg, SAVEDATA);

    if (dst_devid)
    {
        /* remain len */
        remainlen = 1+(2+mystrlen(dst_devid))+1+(2+json_len);
        WriteRemainlen(pkg, remainlen);
        /* translate address flag */
        WriteByte(pkg, 0x80);
        /* dst devid */
        WriteStr(pkg, dst_devid);
    }
    else
    {
        /* remain len */
        remainlen = 1+1+(2+json_len);
        WriteRemainlen(pkg, remainlen);
        /* translate address flag */
        WriteByte(pkg, 0x00);
    }
    /* json flag */
    WriteByte(pkg, type);
    /* json */ 
    WriteStr(pkg, json_out);
    hx_free(json_out);

    return pkg;
}

/* 
 * 定义3种JSON格式的打包和解包函数，
 * 每种格式都支持int,double和char*三种类型
 */
MKFUN_PACKET_SAVE_DATA_TYPE1(int, Int, Number)
MKFUN_PACKET_SAVE_DATA_TYPE1(double, Double, Number)
MKFUN_PACKET_SAVE_DATA_TYPE1(const char*, String, String)

MKFUN_PACKET_SAVE_DATA_TYPE2(int, Int, Number)
MKFUN_PACKET_SAVE_DATA_TYPE2(double, Double, Number)
MKFUN_PACKET_SAVE_DATA_TYPE2(const char*, String, String)

MKFUN_PACKET_SAVE_DATA_TYPE3(int, Int, Number)
MKFUN_PACKET_SAVE_DATA_TYPE3(double, Double, Number)
MKFUN_PACKET_SAVE_DATA_TYPE3(const char*, String, String)

MKFUN_PACKET_SAVE_DATA(int, Int)
MKFUN_PACKET_SAVE_DATA(double, Double)
MKFUN_PACKET_SAVE_DATA(const char*, String)

/* 解包函数 */
MKFUN_UNPACK_SAVE_DATA_TYPE1(int, Int, Number)
UNPACK_SAVE_DATA_TYPE1_END_NUMBER(int)
MKFUN_UNPACK_SAVE_DATA_TYPE1(double, Double, Number)
UNPACK_SAVE_DATA_TYPE1_END_NUMBER(double)
MKFUN_UNPACK_SAVE_DATA_TYPE1(char*, String, String)
UNPACK_SAVE_DATA_TYPE1_END_STRING

MKFUN_UNPACK_SAVE_DATA_TYPE2(int, Int, Number)
UNPACK_SAVE_DATA_TYPE23_END_NUMBER(int)
MKFUN_UNPACK_SAVE_DATA_TYPE2(double, Double, Number)
UNPACK_SAVE_DATA_TYPE23_END_NUMBER(double)
MKFUN_UNPACK_SAVE_DATA_TYPE2(char*, String, String)
UNPACK_SAVE_DATA_TYPE23_END_STRING()

MKFUN_UNPACK_SAVE_DATA_TYPE3(int, Int, Number)
UNPACK_SAVE_DATA_TYPE23_END_NUMBER(int)
MKFUN_UNPACK_SAVE_DATA_TYPE3(double, Double, Number)
UNPACK_SAVE_DATA_TYPE23_END_NUMBER(double)
MKFUN_UNPACK_SAVE_DATA_TYPE3(char*, String, String)
UNPACK_SAVE_DATA_TYPE23_END_STRING()

MKFUN_UNPACK_SAVE_DATA(int, Int)
MKFUN_UNPACK_SAVE_DATA(double, Double)
MKFUN_UNPACK_SAVE_DATA(char*, String)

int32 UnpackSavedataAck(EdpPacket* pkg, char** json_ack)
{
    uint32 remainlen;
	uint8 flag;
	uint16 json_len;

    if (ReadRemainlen(pkg, &remainlen))
	{
		return ERR_UNPACK_SAVEDATA_ACK;
	}

    
    if (ReadByte(pkg, &flag))
	{
		return ERR_UNPACK_SAVEDATA_ACK;
	}

    
    if (ReadUint16(pkg, &json_len))
	{
		return ERR_UNPACK_SAVEDATA_ACK;
	}

    if (ReadBytes(pkg, (uint8**)(json_ack), json_len))
	{
		return ERR_UNPACK_SAVEDATA_ACK;
	}

    return 0;
}

/* sava_data bin (C->S) */
EdpPacket* PacketSavedataBin(const char* dst_devid, cJSON* desc_obj, uint8* bin_data, uint32 bin_len)
{
    EdpPacket* pkg;

    uint32 remainlen;
    /* check arguments */
    char* desc_out  = cJSON_Print(desc_obj);
    uint32 desc_len = mystrlen(desc_out);
			

    if (desc_len > (0x01 << 16) || bin_len > (3 * (0x01 << 20)) 
            /* desc < 2^16 && bin_len < 3M*/
            || cJSON_GetObjectItem(desc_obj, "ds_id") == 0)  
            /* desc_obj MUST has ds_id */
    {
        hx_free(desc_out);
        return 0;
    }

	
    pkg = NewBuffer();
	/* msg type */
	WriteByte(pkg, SAVEDATA);
    
	if (dst_devid)
    {
        /* remain len */
        remainlen = 1+(2+mystrlen(dst_devid))+1+(2+desc_len)+(4+bin_len);
        WriteRemainlen(pkg, remainlen);
        /* translate address flag */
        WriteByte(pkg, 0x80);
        /* dst devid */
        WriteStr(pkg, dst_devid);
    }
    else
    {
        /* remain len */
        remainlen = 1+1+(2+desc_len)+(4+bin_len);
        WriteRemainlen(pkg, remainlen);
        /* translate address flag */
        WriteByte(pkg, 0x00);
    }
    /* bin flag */
    WriteByte(pkg, 0x02);
    /* desc */ 
    WriteStr(pkg, desc_out);
    hx_free(desc_out);
    /* bin data */
    WriteUint32(pkg, bin_len);
    WriteBytes(pkg, bin_data, bin_len);

    return pkg;
}

EdpPacket* PacketSavedataBinStr(const char* dst_devid,  const char* desc_str, const uint8* bin_data, uint32 bin_len)
{
	EdpPacket* pkg;
    uint32 remainlen;
    /* check arguments */
    uint32 desc_len = mystrlen(desc_str);

    if (desc_len > (0x01 << 16) || bin_len > (3 * (0x01 << 20)))           
    { /* desc < 2^16 && bin_len < 3M*/
        return 0;
    }
    pkg = NewBuffer();
	/* msg type */
	WriteByte(pkg, SAVEDATA);
    if (dst_devid)
    {
        /* remain len */
        remainlen = 1+(2+mystrlen(dst_devid))+1+(2+desc_len)+(4+bin_len);
        WriteRemainlen(pkg, remainlen);
        /* translate address flag */
        WriteByte(pkg, 0x80);
        /* dst devid */
        WriteStr(pkg, dst_devid);
    }
    else
    {
        /* remain len */
        remainlen = 1+1+(2+desc_len)+(4+bin_len);
        WriteRemainlen(pkg, remainlen);
        /* translate address flag */
        WriteByte(pkg, 0x00);
    }
    /* bin flag */
    WriteByte(pkg, 0x02);
    /* desc */ 
    WriteStr(pkg, desc_str);
    /* bin data */
    WriteUint32(pkg, bin_len);
    WriteBytes(pkg, bin_data, bin_len);
    return pkg;
}

EdpPacket* PacketCmdResp(const char* cmdid, uint16 cmdid_len, const char* resp, uint32 resp_len)
{
	unsigned remainlen;

    EdpPacket* send_pkg = NewBuffer();
    /* 6 = 2 + 4 = len(cmdid_len) + len(resp_len) */
    remainlen = cmdid_len + resp_len + (resp_len ? 6 : 2);

    WriteByte(send_pkg, CMDRESP);
    WriteRemainlen(send_pkg, remainlen);
    WriteUint16(send_pkg, cmdid_len);
    WriteBytes(send_pkg, cmdid, cmdid_len);

    if (resp_len)
	{
		WriteUint32(send_pkg, resp_len);
		WriteBytes(send_pkg, resp, resp_len);
    }

    return send_pkg;
}
/* ping (C->S) */
EdpPacket* PacketPing(void)
{
    EdpPacket* pkg = NewBuffer();
	if(!pkg->_data)
	{
		PrintLine("NewBuffer error");
		return NULL;
	}
	/* msg type */
	WriteByte(pkg, PINGREQ);
    /* remain len */
    WriteRemainlen(pkg, 0);

    return pkg;
}
/*---------------------------------------------------------------------------*/
/* recv stream to a edp packet (S->C) */
EdpPacket* GetEdpPacket(RecvBuffer* buf)
{
    //assert(buf->_read_pos == 0);
    EdpPacket* pkg = 0;
    int32 flag     = IsPkgComplete(buf);  

    if (flag <= 0)
	{		
		return pkg;
	}

    pkg = NewBuffer();
    WriteBytes(pkg, buf->_data, flag);

    /* shrink buffer */
    mymemmove(buf->_data, buf->_data + flag, buf->_write_pos - flag);
    buf->_write_pos -= flag;
    
	return pkg;
}
/* is the recv buffer has a complete edp packet? */
int32 IsPkgComplete(RecvBuffer* buf)
{
    uint8* data = buf->_data;
    uint32 data_len = buf->_write_pos;

	/* recevie remaining len */
	uint32 multiplier = 1;
	uint32 len_val = 0;
	uint32 len_len = 1;
	uint8* pdigit = data;
	uint32 pkg_total_len;

	if (data_len <= 1) {
		return 0;   /* continue receive */
	}


	do {
		if (len_len > 4) {
			return -1;  /* protocol error; */
		}
		if (len_len > data_len - 1) {
			return 0;   /* continue receive */
		}
		len_len++;
		pdigit++;
		len_val += ((*pdigit) & 0x7f) * multiplier;
		multiplier *= 0x80;
	}while(((*pdigit) & 0x80) != 0);

	 pkg_total_len = len_len + len_val;
	/* receive payload */
	if (pkg_total_len <= (uint32)data_len)
	{
//#ifdef _DEBUG
	//	_hx_printf("a complete packet len:%d\n", pkg_total_len);
//#endif
		return pkg_total_len;   /* all data for this pkg is read */
	}
	else
	{
		return 0;   /* continue receive */
	}
}
/* get edp packet type, client should use this type to invoke Unpack??? function */
uint8 EdpPacketType(EdpPacket* pkg)
{
    uint8 mtype = 0x00;

    ReadByte(pkg, &mtype);

    return mtype;
}
/* connect_resp (S->C)*/
int32 UnpackConnectResp(EdpPacket* pkg)
{
    uint8 flag, rtn;
    uint32 remainlen;

    if (ReadRemainlen(pkg, &remainlen))
	{
		return ERR_UNPACK_CONNRESP_REMAIN;
	}
    
	if (ReadByte(pkg, &flag))
	{
		return ERR_UNPACK_CONNRESP_FLAG;
	}

    if (ReadByte(pkg, &rtn))
	{
		return ERR_UNPACK_CONNRESP_RTN;
	}
	    

    return (int32)rtn;
}
/* push_data (S->C) */
int32 UnpackPushdata(EdpPacket* pkg, char** src_devid, char** data, uint32* data_len)
{
    uint32 remainlen;

    if (ReadRemainlen(pkg, &remainlen))
        return ERR_UNPACK_PUSHD_REMAIN;
    if (ReadStr(pkg, src_devid))
        return ERR_UNPACK_PUSHD_DEVID;
    remainlen -= (2 + mystrlen(*src_devid));
    if (ReadBytes(pkg, (uint8**)data, remainlen))
        return ERR_UNPACK_PUSHD_DATA;
    *data_len = remainlen;

    //assert(pkg->_read_pos == pkg->_write_pos);
    return 0;
}
/* save_data (S->C) */
int32 UnpackSavedata(EdpPacket* pkg, char** src_devid, uint8* jb_flag)
{
    uint32 remainlen;
    uint8 ta_flag;
    
	if (ReadRemainlen(pkg, &remainlen))
        return ERR_UNPACK_SAVED_REMAIN;
    /* translate address flag */
    if (ReadByte(pkg, &ta_flag)) 
        return ERR_UNPACK_SAVED_TANSFLAG;
    if (ta_flag == 0x80)
    {
        if (ReadStr(pkg, src_devid))
            return ERR_UNPACK_SAVED_DEVID;
    }
    else
    {
        *src_devid = 0;
    }
    /* json or bin */
    if (ReadByte(pkg, jb_flag)) 
        return ERR_UNPACK_SAVED_DATAFLAG;
    return 0;
}
int32 UnpackSavedataJson(EdpPacket* pkg, cJSON** json_obj)
{
    char* json_str;

    if (ReadStr(pkg, &json_str))
        return ERR_UNPACK_SAVED_JSON;
    *json_obj = cJSON_Parse(json_str);
    hx_free(json_str);
    if (*json_obj == 0)
        return ERR_UNPACK_SAVED_PARSEJSON;
    //assert(pkg->_read_pos == pkg->_write_pos);
    return 0;
}

int32 UnpackSavedataBin(EdpPacket* pkg, cJSON** desc_obj, 
        uint8** bin_data, uint32* bin_len)
{
    char* desc_str;
    
	if (ReadStr(pkg, &desc_str))
        return ERR_UNPACK_SAVED_BIN_DESC;
    *desc_obj = cJSON_Parse(desc_str); 
    hx_free(desc_str);
    
	if (*desc_obj == 0)
        return ERR_UNPACK_SAVED_PARSEDESC;
    
	if (ReadUint32(pkg, bin_len))
        return ERR_UNPACK_SAVED_BINLEN;

    if (ReadBytes(pkg, bin_data, *bin_len))
        return ERR_UNPACK_SAVED_BINDATA;
    //assert(pkg->_read_pos == pkg->_write_pos);
    return 0;
}

int32 UnpackSavedataBinStr(EdpPacket* pkg, char** desc_str,uint8** bin_data, uint32* bin_len)
{
    if (ReadStr(pkg, desc_str))
        return ERR_UNPACK_SAVED_BIN_DESC;
    if (ReadUint32(pkg, bin_len))
        return ERR_UNPACK_SAVED_BINLEN;
    if (ReadBytes(pkg, bin_data, *bin_len))
        return ERR_UNPACK_SAVED_BINDATA;

    //assert(pkg->_read_pos == pkg->_write_pos);

    return 0;
}

int32 UnpackCmdReq(EdpPacket* pkg, char** cmdid, uint16* cmdid_len, char** req, uint32* req_len)
{
    uint32 remainlen;
	int rc;
    
	if (ReadRemainlen(pkg, &remainlen))
		return ERR_UNPACK_CMDREQ;

    rc = ReadUint16(pkg, cmdid_len);
    if (rc) 
        return rc;
    if (ReadBytes(pkg, (uint8**)cmdid, *cmdid_len))
        return ERR_UNPACK_CMDREQ;

    rc = ReadUint32(pkg, req_len);
    if (rc) 
        return rc;
    if (ReadBytes(pkg, (uint8**)req, *req_len))
        return ERR_UNPACK_CMDREQ;

    //assert(pkg->_read_pos == pkg->_write_pos);
    return 0;
}

/* ping_resp (S->C) */
int32 UnpackPingResp(EdpPacket* pkg)
{
    uint32 remainlen;
    
	if (ReadRemainlen(pkg, &remainlen))
        return ERR_UNPACK_PING_REMAIN;
    //assert(pkg->_read_pos == pkg->_write_pos);
    return 0;
}

/* sava_data (C->S) */
EdpPacket* PacketSavedataSimpleString(const char* dst_devid, const char* input)
{
    EdpPacket* pkg = NewBuffer();
    uint32 remainlen;
    uint32 input_len = mystrlen(input);
    /* msg type */
    WriteByte(pkg, SAVEDATA);
    if (dst_devid)
    {
        /* remain len */
        remainlen = 1+(2+mystrlen(dst_devid))+1+(2+input_len);
        WriteRemainlen(pkg, remainlen);
        /* translate address flag */
        WriteByte(pkg, 0x80);
        /* dst devid */
        WriteStr(pkg, dst_devid);
    }
    else
    {
        /* remain len */
        remainlen = 1+1+(2+input_len);
        WriteRemainlen(pkg, remainlen);
        /* translate address flag */
        WriteByte(pkg, 0x00);
    }
    /* json flag */
    WriteByte(pkg, kTypeString);
    /* json */ 
    WriteStr(pkg, input);

    return pkg;
}

int32 UnpackSavedataSimpleString(EdpPacket* pkg, char** output)
{
    if (ReadStr(pkg, output))
        return ERR_UNPACK_SAVED_JSON;
    return 0;
}




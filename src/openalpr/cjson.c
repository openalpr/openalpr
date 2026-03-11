/*
  Copyright (c) 2009 Dave Gamble

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

/* cJSON */
/* JSON parser in C. */

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <locale.h>
#include "cjson.h"

static const char *ep;

const char *cJSON_GetErrorPtr(void) {return ep;}

static int cJSON_strcasecmp(const char *s1,const char *s2)
{
	if (!s1) return (s1==s2)?0:1;if (!s2) return 1;
	for(; tolower(*s1) == tolower(*s2); ++s1, ++s2)	if(*s1 == 0)	return 0;
	return tolower(*(const unsigned char *)s1) - tolower(*(const unsigned char *)s2);
}

static void *(*cJSON_malloc)(size_t sz) = malloc;
static void (*cJSON_free)(void *ptr) = free;
{
      size_t len;
      char* copy;
      len = strlen(str) + 1;
static int cJSON_strcasecmp(const char *s1,const char *s2)
      if (!(copy = (char*)cJSON_malloc(len))) return 0;
	if (!s1) return (s1==s2)?0:1;if (!s2) return 1;
      memcpy(copy,str,len);
	for(; tolower(*s1) == tolower(*s2); ++s1, ++s2)	if(*s1 == 0)	return 0;
void cJSON_InitHooks(cJSON_Hooks* hooks)
    if (!hooks) { /* Reset hooks */
    }

static void *(*cJSON_malloc)(size_t sz) = malloc;
static char* cJSON_strdup(const char* str)
      size_t len;
static void (*cJSON_free)(void *ptr) = free;
      char* copy;
{
	if (node) memset(node,0,sizeof(cJSON));

      len = strlen(str) + 1;
/* Delete a cJSON structure. */
      if (!(copy = (char*)cJSON_malloc(len))) return 0;
{
      memcpy(copy,str,len);
	cJSON *next;
	while (c)
}
	{
    if (!hooks) { /* Reset hooks */
		next=c->next;
        cJSON_malloc = malloc;
void cJSON_InitHooks(cJSON_Hooks* hooks)
        cJSON_free = free;
		if (!(c->type&cJSON_IsReference) && c->child) cJSON_Delete(c->child);
		if (!(c->type&cJSON_IsReference) && c->valuestring) cJSON_free(c->valuestring);
    }
		if (c->string) cJSON_free(c->string);
		cJSON_free(c);
		c=next;
	}
}

	cJSON_malloc = (hooks->malloc_fn)?hooks->malloc_fn:malloc;
/* Parse the input text to generate a number, and populate the result into item. */
static cJSON *cJSON_New_Item(void)
	cJSON_free	 = (hooks->free_fn)?hooks->free_fn:free;
static const char *parse_number(cJSON *item,const char *num)
{
	cJSON* node = (cJSON*)cJSON_malloc(sizeof(cJSON));
	double n=0,sign=1,scale=0;int subscale=0,signsubscale=1;
	if (node) memset(node,0,sizeof(cJSON));
	if (*num=='-') sign=-1,num++;	/* Has sign? */
	if (*num=='0') num++;			/* is zero */
	if (*num>='1' && *num<='9')	do	n=(n*10.0)+(*num++ -'0');	while (*num>='0' && *num<='9');	/* Number? */
	if (*num=='.' || *num==','  && num[1]>='0' && num[1]<='9') {num++;		do	n=(n*10.0)+(*num++ -'0'),scale--; while (*num>='0' && *num<='9');}	/* Fractional part? */
	if (*num=='e' || *num=='E')		/* Exponent? */
	{	num++;if (*num=='+') num++;	else if (*num=='-') signsubscale=-1,num++;		/* With sign? */
		while (*num>='0' && *num<='9') subscale=(subscale*10)+(*num++ - '0');	/* Number? */
	}

	n=sign*n*pow(10.0,(scale+subscale*signsubscale));	/* number = +/- number.fraction * 10^+/- exponent */
	item->valuedouble=n;
	while (c)
	item->valueint=(int)n;
	{
	item->type=cJSON_Number;
		next=c->next;
	return num;
		if (!(c->type&cJSON_IsReference) && c->child) cJSON_Delete(c->child);
}
		if (!(c->type&cJSON_IsReference) && c->valuestring) cJSON_free(c->valuestring);
/* Render the number nicely from the given item into a string. */
		if (!(c->type&cJSON_StringIsConst) && c->string) cJSON_free(c->string);
static char *print_number(cJSON *item)
		cJSON_free(c);
{
		c=next;
    char * locale = setlocale(LC_ALL, NULL);
    setlocale(LC_NUMERIC, "C");
/* Parse the input text to generate a number, and populate the result into item. */
	char *str;
static const char *parse_number(cJSON *item,const char *num)
	double d=item->valuedouble;
	if (fabs(((double)item->valueint)-d)<=DBL_EPSILON && d<=INT_MAX && d>=INT_MIN)
	{
	double n=0,sign=1,scale=0;int subscale=0,signsubscale=1;
		str=(char*)cJSON_malloc(21);	/* 2^64+1 can be represented in 21 chars. */
		if (str) sprintf(str,"%d",item->valueint);
	if (*num=='-') sign=-1,num++;	/* Has sign? */
	}
	if (*num=='0') num++;			/* is zero */
	else
	if (*num>='1' && *num<='9')	do	n=(n*10.0)+(*num++ -'0');	while (*num>='0' && *num<='9');	/* Number? */
	{
	if (*num=='.' && num[1]>='0' && num[1]<='9') {num++;		do	n=(n*10.0)+(*num++ -'0'),scale--; while (*num>='0' && *num<='9');}	/* Fractional part? */
		str=(char*)cJSON_malloc(64);	/* This is a nice tradeoff. */
	if (*num=='e' || *num=='E')		/* Exponent? */
		if (str)
	{	num++;if (*num=='+') num++;	else if (*num=='-') signsubscale=-1,num++;		/* With sign? */
		{
		while (*num>='0' && *num<='9') subscale=(subscale*10)+(*num++ - '0');	/* Number? */
			if (fabs(floor(d)-d)<=DBL_EPSILON && fabs(d)<1.0e60)sprintf(str,"%.0f",d);
			else if (fabs(d)<1.0e-6 || fabs(d)>1.0e9)			sprintf(str,"%e",d);
			else												sprintf(str,"%f",d);
	}

	n=sign*n*pow(10.0,(scale+subscale*signsubscale));	/* number = +/- number.fraction * 10^+/- exponent */
	setlocale(LC_NUMERIC, locale);
static unsigned parse_hex4(const char *str)
	item->type=cJSON_Number;
	return num;
	if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;
static int pow2gt (int x)	{	--x;	x|=x>>1;	x|=x>>2;	x|=x>>4;	x|=x>>8;	x|=x>>16;	return x+1;	}
typedef struct {char *buffer; int length; int offset; } printbuffer;
	h=h<<4;str++;
	if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;
static char* ensure(printbuffer *p,int needed)
	h=h<<4;str++;
{
	if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;
	char *newbuffer;int newsize;
	h=h<<4;str++;
	if (!p || !p->buffer) return 0;
	if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;
	needed+=p->offset;
	return h;
	if (needed<=p->length) return p->buffer+p->offset;
static const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
static const char *parse_string(cJSON *item,const char *str)
{
	const char *ptr=str+1;char *ptr2;char *out;int len=0;unsigned uc,uc2;
	if (*str!='\"') {ep=str;return 0;}	/* not a string! */
	newsize=pow2gt(needed);

	newbuffer=(char*)cJSON_malloc(newsize);
	while (*ptr!='\"' && *ptr && ++len) if (*ptr++ == '\\') ptr++;	/* Skip escaped quotes. */
	if (!newbuffer) {cJSON_free(p->buffer);p->length=0,p->buffer=0;return 0;}

	if (newbuffer) memcpy(newbuffer,p->buffer,p->length);
	out=(char*)cJSON_malloc(len+1);	/* This is how long we need for the string, roughly. */
	cJSON_free(p->buffer);
	p->length=newsize;
	ptr=str+1;ptr2=out;
	p->buffer=newbuffer;
	while (*ptr!='\"' && *ptr)
	return newbuffer+p->offset;
	{
		if (*ptr!='\\') *ptr2++=*ptr++;
		else
static int update(printbuffer *p)
		{
			ptr++;
			switch (*ptr)
	if (!p || !p->buffer) return 0;
			{
	str=p->buffer+p->offset;
				case 'b': *ptr2++='\b';	break;
	return p->offset+strlen(str);
				case 'f': *ptr2++='\f';	break;
}
				case 'n': *ptr2++='\n';	break;
				case 'r': *ptr2++='\r';	break;
				case 't': *ptr2++='\t';	break;
				case 'u':	 /* transcode utf16 to utf8. */
					uc=parse_hex4(ptr+1);ptr+=4;	/* get the unicode char. */

/* Render the number nicely from the given item into a string. */
					if ((uc>=0xDC00 && uc<=0xDFFF) || uc==0)	break;	/* check for invalid.	*/
static char *print_number(cJSON *item,printbuffer *p)

{
					if (uc>=0xD800 && uc<=0xDBFF)	/* UTF16 surrogate pairs.	*/
	char *str=0;
					{
	double d=item->valuedouble;
						if (ptr[1]!='\\' || ptr[2]!='u')	break;	/* missing second-half of surrogate.	*/
	if (d==0)
						uc2=parse_hex4(ptr+3);ptr+=6;
	{
						if (uc2<0xDC00 || uc2>0xDFFF)		break;	/* invalid second-half of surrogate.	*/
		if (p)	str=ensure(p,2);
						uc=0x10000 + (((uc&0x3FF)<<10) | (uc2&0x3FF));
		else	str=(char*)cJSON_malloc(2);	/* special case for 0. */
					}
		if (str) strcpy(str,"0");

	}
					len=4;if (uc<0x80) len=1;else if (uc<0x800) len=2;else if (uc<0x10000) len=3; ptr2+=len;
	else if (fabs(((double)item->valueint)-d)<=DBL_EPSILON && d<=LLONG_MAX && d>=LLONG_MIN)

	{
					switch (len) {
		if (p)	str=ensure(p,64);
						case 4: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
		else	str=(char*)cJSON_malloc(64);
						case 3: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
		if (str)	sprintf(str,"%lld",(long long) item->valueint);
						case 2: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
	}
						case 1: *--ptr2 =(uc | firstByteMark[len]);
	else
					}
	{
					ptr2+=len;
		if (p)	str=ensure(p,64);
					break;
		else	str=(char*)cJSON_malloc(64);	/* This is a nice tradeoff. */
				default:  *ptr2++=*ptr; break;
		if (str)
			}
		{
			ptr++;
			if (fpclassify(d) != FP_ZERO && !isnormal(d))				sprintf(str,"null");
		}
			else if (fabs(floor(d)-d)<=DBL_EPSILON && fabs(d)<1.0e60)	sprintf(str,"%.0f",d);
	}
			else if (fabs(d)<1.0e-6 || fabs(d)>1.0e9)					sprintf(str,"%e",d);
	*ptr2=0;
			else														sprintf(str,"%f",d);
	if (*ptr=='\"') ptr++;
		}
	item->valuestring=out;
	item->type=cJSON_String;
	return ptr;
}

static unsigned parse_hex4(const char *str)
/* Render the cstring provided to an escaped version that can be printed. */
{
static char *print_string_ptr(const char *str)
	unsigned h=0;
{
	if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;
	const char *ptr;char *ptr2,*out;int len=0;unsigned char token;
	h=h<<4;str++;

	if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;
	if (!str) return cJSON_strdup("");
	h=h<<4;str++;
	ptr=str;while ((token=*ptr) && ++len) {if (strchr("\"\\\b\f\n\r\t",token)) len++; else if (token<32) len+=5;ptr++;}
	if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;

	h=h<<4;str++;
	out=(char*)cJSON_malloc(len+3);
	if (*str>='0' && *str<='9') h+=(*str)-'0'; else if (*str>='A' && *str<='F') h+=10+(*str)-'A'; else if (*str>='a' && *str<='f') h+=10+(*str)-'a'; else return 0;
	if (!out) return 0;
static const char *parse_string(cJSON *item,const char *str,const char **ep)
	ptr2=out;ptr=str;
	{
	const char *ptr=str+1,*end_ptr=str+1;char *ptr2;char *out;int len=0;unsigned uc,uc2;
		if ((unsigned char)*ptr>31 && *ptr!='\"' && *ptr!='\\') *ptr2++=*ptr++;
		else
	while (*end_ptr!='\"' && *end_ptr && ++len) if (*end_ptr++ == '\\') end_ptr++;	/* Skip escaped quotes. */
		{
			switch (token=*ptr++)
	out=(char*)cJSON_malloc(len+1);	/* This is how long we need for the string, roughly. */
			{
	if (!out) return 0;
				case '\\':	*ptr2++='\\';	break;
	item->valuestring=out; /* assign here so out will be deleted during cJSON_Delete() later */
				case '\"':	*ptr2++='\"';	break;
	item->type=cJSON_String;
	ptr=str+1;ptr2=out;
	while (ptr < end_ptr)
	{
		else
			switch (*ptr)
		{
			{
				default: sprintf(ptr2,"u%04x",token);ptr2+=5;	break;	/* escape and print */
				case 'b': *ptr2++='\b';	break;
			}
				case 'f': *ptr2++='\f';	break;
		}
				case 'n': *ptr2++='\n';	break;
	}
				case 'r': *ptr2++='\r';	break;
}
				case 't': *ptr2++='\t';	break;
static char *print_string(cJSON *item)	{return print_string_ptr(item->valuestring);}
				case 'u':	 /* transcode utf16 to utf8. */
static const char *parse_value(cJSON *item,const char *value);
					uc=parse_hex4(ptr+1);ptr+=4;	/* get the unicode char. */
static char *print_value(cJSON *item,int depth,int fmt);
					if (ptr >= end_ptr) {*ep=str;return 0;}	/* invalid */
static const char *parse_array(cJSON *item,const char *value);
					
static char *print_array(cJSON *item,int depth,int fmt);
					if ((uc>=0xDC00 && uc<=0xDFFF) || uc==0)    {*ep=str;return 0;}	/* check for invalid.   */
static const char *parse_object(cJSON *item,const char *value);
					
static char *print_object(cJSON *item,int depth,int fmt);
					if (uc>=0xD800 && uc<=0xDBFF)	/* UTF16 surrogate pairs.	*/

					{
/* Utility to jump whitespace and cr/lf */
						if (ptr+6 > end_ptr)    {*ep=str;return 0;}	/* invalid */
static const char *skip(const char *in) {while (in && *in && (unsigned char)*in<=32) in++; return in;}
						if (ptr[1]!='\\' || ptr[2]!='u')    {*ep=str;return 0;}	/* missing second-half of surrogate.    */

						uc2=parse_hex4(ptr+3);ptr+=6;
/* Parse an object - create a new root, and populate. */
						if (uc2<0xDC00 || uc2>0xDFFF)       {*ep=str;return 0;}	/* invalid second-half of surrogate.    */
cJSON *cJSON_ParseWithOpts(const char *value,const char **return_parse_end,int require_null_terminated)
						uc=0x10000 + (((uc&0x3FF)<<10) | (uc2&0x3FF));
{
	const char *end=0;
	cJSON *c=cJSON_New_Item();
	ep=0;
	if (!c) return 0;       /* memory fail */

					len=4;if (uc<0x80) len=1;else if (uc<0x800) len=2;else if (uc<0x10000) len=3; ptr2+=len;
	end=parse_value(c,skip(value));
	if (!end)	{cJSON_Delete(c);return 0;}	/* parse failure. ep is set. */

					switch (len) {
	/* if we require null-terminated JSON without appended garbage, skip and then check for a null terminator */
						case 4: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
	if (require_null_terminated) {end=skip(end);if (*end) {cJSON_Delete(c);ep=end;return 0;}}
						case 3: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
	if (return_parse_end) *return_parse_end=end;
						case 2: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
	return c;
						case 1: *--ptr2 =(uc | firstByteMark[len]);
}
/* Default options for cJSON_Parse */
cJSON *cJSON_Parse(const char *value) {return cJSON_ParseWithOpts(value,0,0);}
				default:  *ptr2++=*ptr; break;

/* Render a cJSON item/entity/structure to text. */
char *cJSON_Print(cJSON *item)				{return print_value(item,0,1);}
char *cJSON_PrintUnformatted(cJSON *item)	{return print_value(item,0,0);}

	*ptr2=0;
/* Parser core - when encountering text, process appropriately. */
	if (*ptr=='\"') ptr++;
static const char *parse_value(cJSON *item,const char *value)
{
	const char *ptr;char *ptr2,*out;int len=0,flag=0;unsigned char token;
	if (!value)						return 0;	/* Fail on null. */

	if (!strncmp(value,"null",4))	{ item->type=cJSON_NULL;  return value+4; }
	if (!str)
static char *print_string_ptr(const char *str,printbuffer *p)
	{
	if (!strncmp(value,"false",5))	{ item->type=cJSON_False; return value+5; }
		if (p)	out=ensure(p,3);
	if (!strncmp(value,"true",4))	{ item->type=cJSON_True; item->valueint=1;	return value+4; }
		else	out=(char*)cJSON_malloc(3);
	if (*value=='\"')				{ return parse_string(item,value); }
		if (!out) return 0;
	if (*value=='-' || (*value>='0' && *value<='9'))	{ return parse_number(item,value); }
		strcpy(out,"\"\"");
	if (*value=='[')				{ return parse_array(item,value); }
		return out;
	if (*value=='{')				{ return parse_object(item,value); }
	}
	ep=value;return 0;	/* failure. */
}
	for (ptr=str;*ptr;ptr++) flag|=((*ptr>0 && *ptr<32)||(*ptr=='\"')||(*ptr=='\\'))?1:0;

	if (!flag)
/* Render a value to text. */
	{
{
		len=ptr-str;
	char *out=0;
		if (p) out=ensure(p,len+3);
	if (!item) return 0;
		else		out=(char*)cJSON_malloc(len+3);
	switch ((item->type)&255)
		if (!out) return 0;
	{
		ptr2=out;*ptr2++='\"';
		case cJSON_False:	out=cJSON_strdup("false");break;
		strcpy(ptr2,str);
		case cJSON_True:	out=cJSON_strdup("true"); break;
		case cJSON_Number:	out=print_number(item);break;
		case cJSON_String:	out=print_string(item);break;
		case cJSON_Array:	out=print_array(item,depth,fmt);break;
		case cJSON_Object:	out=print_object(item,depth,fmt);break;
	}
	ptr=str;while ((token=*ptr) && ++len) {if (strchr("\"\\\b\f\n\r\t",token)) len++; else if (token<32) len+=5;ptr++;}
}
	
static const char *parse_array(cJSON *item,const char *value)
	if (p)	out=ensure(p,len+3);
{
	else	out=(char*)cJSON_malloc(len+3);
	cJSON *child;
	if (!out) return 0;
	if (*value!='[')	{ep=value;return 0;}	/* not an array! */


	ptr2=out;ptr=str;
	item->type=cJSON_Array;
	*ptr2++='\"';
	value=skip(value+1);
	while (*ptr)
	if (*value==']') return value+1;	/* empty array. */
	{

		if ((unsigned char)*ptr>31 && *ptr!='\"' && *ptr!='\\') *ptr2++=*ptr++;
	item->child=child=cJSON_New_Item();
		else
	if (!item->child) return 0;		 /* memory fail */
		{
	value=skip(parse_value(child,skip(value)));	/* skip any spacing, get the value. */
			*ptr2++='\\';
	if (!value) return 0;
			switch (token=*ptr++)

			{
	while (*value==',')
				case '\\':	*ptr2++='\\';	break;
	{
				case '\"':	*ptr2++='\"';	break;
		cJSON *new_item;
				case '\b':	*ptr2++='b';	break;
		if (!(new_item=cJSON_New_Item())) return 0; 	/* memory fail */
				case '\f':	*ptr2++='f';	break;
		child->next=new_item;new_item->prev=child;child=new_item;
				case '\n':	*ptr2++='n';	break;
		value=skip(parse_value(child,skip(value+1)));
				case '\r':	*ptr2++='r';	break;
static const char *parse_value(cJSON *item,const char *value,const char **ep);
		if (!value) return 0;	/* memory fail */
static char *print_value(cJSON *item,int depth,int fmt,printbuffer *p);
				case '\t':	*ptr2++='t';	break;
static const char *parse_array(cJSON *item,const char *value,const char **ep);
				default: sprintf(ptr2,"u%04x",token);ptr2+=5;	break;	/* escape and print */
static char *print_array(cJSON *item,int depth,int fmt,printbuffer *p);
	if (*value==']') return value+1;	/* end of array */
static const char *parse_object(cJSON *item,const char *value,const char **ep);
	*ptr2++='\"';*ptr2++=0;
static char *print_object(cJSON *item,int depth,int fmt,printbuffer *p);
	char **entries;
	char *out=0,*ptr,*ret;int len=5;
	cJSON *child=item->child;
	int numentries=0,i=0,fail=0;
	while (child) numentries++,child=child->next;
	/* Explicitly handle numentries==0 */
/* Utility to jump whitespace and cr/lf */
	if (!numentries)
static const char *skip(const char *in) {while (in && *in && (unsigned char)*in<=32) in++; return in;}
	{
		out=(char*)cJSON_malloc(3);
		if (out) strcpy(out,"[]");
		return out;
	}
	/* Allocate an array to hold the values for each */
cJSON *cJSON_ParseWithOpts(const char *value,const char **return_parse_end,int require_null_terminated)
	entries=(char**)cJSON_malloc(numentries*sizeof(char*));
	memset(entries,0,numentries*sizeof(char*));
	/* Retrieve all the results: */
	child=item->child;
	while (child && !fail)
	{
	const char *end=0,**ep=return_parse_end?return_parse_end:&global_ep;
		ret=print_value(child,depth+1,fmt);
		entries[i++]=ret;
	*ep=0;
		if (ret) len+=strlen(ret)+2+(fmt?1:0); else fail=1;
		child=child->next;

	end=parse_value(c,skip(value),ep);
	/* If we didn't fail, try to malloc the output string */
	if (!end)	{cJSON_Delete(c);return 0;}	/* parse failure. ep is set. */
	if (!fail) out=(char*)cJSON_malloc(len);

	/* If that fails, we fail. */
	/* if we require null-terminated JSON without appended garbage, skip and then check for a null terminator */
	if (!out) fail=1;
	if (require_null_terminated) {end=skip(end);if (*end) {cJSON_Delete(c);*ep=end;return 0;}}

	if (return_parse_end) *return_parse_end=end;
	/* Handle failure. */
	if (fail)
	{
		for (i=0;i<numentries;i++) if (entries[i]) cJSON_free(entries[i]);
		cJSON_free(entries);
		return 0;
/* Default options for cJSON_Parse */
cJSON *cJSON_Parse(const char *value) {return cJSON_ParseWithOpts(value,0,0);}
char *cJSON_Print(cJSON *item)				{return print_value(item,0,1,0);}
	/* Compose the output array. */
	*out='[';

	ptr=out+1;*ptr=0;
char *cJSON_PrintBuffered(cJSON *item,int prebuffer,int fmt)
	for (i=0;i<numentries;i++)
		strcpy(ptr,entries[i]);ptr+=strlen(entries[i]);
	printbuffer p;
		if (i!=numentries-1) {*ptr++=',';if(fmt)*ptr++=' ';*ptr=0;}
	p.buffer=(char*)cJSON_malloc(prebuffer);
		cJSON_free(entries[i]);
	p.length=prebuffer;
	p.offset=0;
	cJSON_free(entries);
	if (!value)						return 0;	/* Fail on null. */
	*ptr++=']';*ptr++=0;
	if (!strncmp(value,"null",4))	{ item->type=cJSON_NULL;  return value+4; }

	if (!strncmp(value,"false",5))	{ item->type=cJSON_False; return value+5; }
static const char *parse_value(cJSON *item,const char *value,const char **ep)
	if (!strncmp(value,"true",4))	{ item->type=cJSON_True; item->valueint=1;	return value+4; }
/* Build an object from the text. */
	if (*value=='\"')				{ return parse_string(item,value,ep); }
{
	if (*value=='-' || (*value>='0' && *value<='9'))	{ return parse_number(item,value); }
	cJSON *child;
	if (*value=='[')				{ return parse_array(item,value,ep); }
	if (*value=='{')				{ return parse_object(item,value,ep); }
	*ep=value;return 0;	/* failure. */
	value=skip(value+1);
	if (!value) return 0;
	child->string=child->valuestring;child->valuestring=0;
	if (*value!=':') {ep=value;return 0;}	/* fail! */
	value=skip(parse_value(child,skip(value+1)));	/* skip any spacing, get the value. */
static char *print_value(cJSON *item,int depth,int fmt,printbuffer *p)
	while (*value==',')
{
		cJSON *new_item;
	char *out=0;
		if (!(new_item=cJSON_New_Item()))	return 0; /* memory fail */
	if (!item) return 0;
		child->next=new_item;new_item->prev=child;child=new_item;
	if (p)
		value=skip(parse_string(child,skip(value+1)));
	{
		if (!value) return 0;
		switch ((item->type)&255)
		child->string=child->valuestring;child->valuestring=0;
		{
		if (*value!=':') {ep=value;return 0;}	/* fail! */
			case cJSON_NULL:	{out=ensure(p,5);	if (out) strcpy(out,"null");	break;}
		value=skip(parse_value(child,skip(value+1)));	/* skip any spacing, get the value. */
			case cJSON_False:	{out=ensure(p,6);	if (out) strcpy(out,"false");	break;}
		if (!value) return 0;
			case cJSON_True:	{out=ensure(p,5);	if (out) strcpy(out,"true");	break;}
	}
			case cJSON_Number:	out=print_number(item,p);break;

			case cJSON_String:	out=print_string(item,p);break;
	if (*value=='}') return value+1;	/* end of array */
			case cJSON_Array:	out=print_array(item,depth,fmt,p);break;
	ep=value;return 0;	/* malformed. */
			case cJSON_Object:	out=print_object(item,depth,fmt,p);break;
}
		}

	}
/* Render an object to text. */
	else
static char *print_object(cJSON *item,int depth,int fmt)
	{
{
		switch ((item->type)&255)
	char **entries=0,**names=0;
		{
	char *out=0,*ptr,*ret,*str;int len=7,i=0,j;
			case cJSON_NULL:	out=cJSON_strdup("null");	break;
	cJSON *child=item->child;
			case cJSON_False:	out=cJSON_strdup("false");break;
	int numentries=0,fail=0;
			case cJSON_True:	out=cJSON_strdup("true"); break;
			case cJSON_Number:	out=print_number(item,0);break;
	while (child) numentries++,child=child->next;
			case cJSON_String:	out=print_string(item,0);break;
			case cJSON_Array:	out=print_array(item,depth,fmt,0);break;
	if (!numentries)
			case cJSON_Object:	out=print_object(item,depth,fmt,0);break;
	{
	item->type=cJSON_Array;
		out=(char*)cJSON_malloc(fmt?depth+4:3);
	value=skip(value+1);
static const char *parse_array(cJSON *item,const char *value,const char **ep)
	if (*value==']') return value+1;	/* empty array. */
		ptr=out;*ptr++='{';
	item->child=child=cJSON_New_Item();
		if (fmt) {*ptr++='\n';for (i=0;i<depth-1;i++) *ptr++='\t';}
	if (!item->child) return 0;		 /* memory fail */
		*ptr++='}';*ptr++=0;
	value=skip(parse_value(child,skip(value),ep));	/* skip any spacing, get the value. */
	}
	if (!value) return 0;
	entries=(char**)cJSON_malloc(numentries*sizeof(char*));
	names=(char**)cJSON_malloc(numentries*sizeof(char*));
	if (!names) {cJSON_free(entries);return 0;}
	memset(entries,0,sizeof(char*)*numentries);
	memset(names,0,sizeof(char*)*numentries);

	while (*value==',')
	/* Collect all the results into our arrays: */
	{
	child=item->child;depth++;if (fmt) len+=depth;
		if (!(new_item=cJSON_New_Item())) return 0; 	/* memory fail */
	while (child)
		child->next=new_item;new_item->prev=child;child=new_item;
	{
		value=skip(parse_value(child,skip(value+1),ep));
		names[i]=str=print_string_ptr(child->string);
		entries[i++]=ret=print_value(child,depth,fmt);
		if (str && ret) len+=strlen(ret)+strlen(str)+2+(fmt?2+depth:0); else fail=1;
	if (*value==']') return value+1;	/* end of array */
static char *print_array(cJSON *item,int depth,int fmt,printbuffer *p)
	{
		for (i=0;i<numentries;i++) {if (names[i]) cJSON_free(names[i]);if (entries[i]) cJSON_free(entries[i]);}
	while (child) numentries++,child=child->next;
	char *out=0,*ptr,*ret;int len=5;
	/* Explicitly handle numentries==0 */
	int numentries=0,i=0,fail=0;
	if (!numentries)
	size_t tmplen=0;
	{
	*out='{';ptr=out+1;if (fmt)*ptr++='\n';*ptr=0;
		if (p)	out=ensure(p,3);
	{
		else	out=(char*)cJSON_malloc(3);
		if (fmt) for (j=0;j<depth;j++) *ptr++='\t';
		if (out) strcpy(out,"[]");
		strcpy(ptr,names[i]);ptr+=strlen(names[i]);
		return out;
		*ptr++=':';if (fmt) *ptr++='\t';
	}
		strcpy(ptr,entries[i]);ptr+=strlen(entries[i]);

		if (fmt) *ptr++='\n';*ptr=0;
	if (p)
	}
	{

		/* Compose the output array. */
	cJSON_free(names);cJSON_free(entries);
		i=p->offset;
	if (fmt) for (i=0;i<depth-1;i++) *ptr++='\t';
		ptr=ensure(p,1);if (!ptr) return 0;	*ptr='[';	p->offset++;
	*ptr++='}';*ptr++=0;
		child=item->child;
	return out;
		while (child && !fail)
}
		{

			print_value(child,depth+1,fmt,p);
/* Get Array size/item / object item. */
			p->offset=update(p);
int    cJSON_GetArraySize(cJSON *array)							{cJSON *c=array->child;int i=0;while(c)i++,c=c->next;return i;}
			if (child->next) {len=fmt?2:1;ptr=ensure(p,len+1);if (!ptr) return 0;*ptr++=',';if(fmt)*ptr++=' ';*ptr=0;p->offset+=len;}
cJSON *cJSON_GetArrayItem(cJSON *array,int item)				{cJSON *c=array->child;  while (c && item>0) item--,c=c->next; return c;}
			child=child->next;
cJSON *cJSON_GetObjectItem(cJSON *object,const char *string)	{cJSON *c=object->child; while (c && cJSON_strcasecmp(c->string,string)) c=c->next; return c;}
		}

		ptr=ensure(p,2);if (!ptr) return 0;	*ptr++=']';*ptr=0;
/* Utility for array list handling. */
		out=(p->buffer)+i;
	else
static void suffix_object(cJSON *prev,cJSON *item) {prev->next=item;item->prev=prev;}
	{
static cJSON *create_reference(cJSON *item) {cJSON *ref=cJSON_New_Item();if (!ref) return 0;memcpy(ref,item,sizeof(cJSON));ref->string=0;ref->type|=cJSON_IsReference;ref->next=ref->prev=0;return ref;}
		/* Allocate an array to hold the values for each */
/* Add item to array/object. */
		entries=(char**)cJSON_malloc(numentries*sizeof(char*));
void   cJSON_AddItemToArray(cJSON *array, cJSON *item)						{cJSON *c=array->child;if (!item) return; if (!c) {array->child=item;} else {while (c && c->next) c=c->next; suffix_object(c,item);}}
		if (!entries) return 0;
void   cJSON_AddItemToObject(cJSON *object,const char *string,cJSON *item)	{if (!item) return; if (item->string) cJSON_free(item->string);item->string=cJSON_strdup(string);cJSON_AddItemToArray(object,item);}
		memset(entries,0,numentries*sizeof(char*));
void	cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item)						{cJSON_AddItemToArray(array,create_reference(item));}
		/* Retrieve all the results: */
void	cJSON_AddItemReferenceToObject(cJSON *object,const char *string,cJSON *item)	{cJSON_AddItemToObject(object,string,create_reference(item));}
		child=item->child;

		while (child && !fail)
cJSON *cJSON_DetachItemFromArray(cJSON *array,int which)			{cJSON *c=array->child;while (c && which>0) c=c->next,which--;if (!c) return 0;
		{
	if (c->prev) c->prev->next=c->next;if (c->next) c->next->prev=c->prev;if (c==array->child) array->child=c->next;c->prev=c->next=0;return c;}
			ret=print_value(child,depth+1,fmt,0);
void   cJSON_DeleteItemFromArray(cJSON *array,int which)			{cJSON_Delete(cJSON_DetachItemFromArray(array,which));}
cJSON *cJSON_DetachItemFromObject(cJSON *object,const char *string) {int i=0;cJSON *c=object->child;while (c && cJSON_strcasecmp(c->string,string)) i++,c=c->next;if (c) return cJSON_DetachItemFromArray(object,i);return 0;}
			if (ret) len+=strlen(ret)+2+(fmt?1:0); else fail=1;
void   cJSON_DeleteItemFromObject(cJSON *object,const char *string) {cJSON_Delete(cJSON_DetachItemFromObject(object,string));}
			child=child->next;

/* Replace array/object items with new ones. */
		
void   cJSON_ReplaceItemInArray(cJSON *array,int which,cJSON *newitem)		{cJSON *c=array->child;while (c && which>0) c=c->next,which--;if (!c) return;
		/* If we didn't fail, try to malloc the output string */
	newitem->next=c->next;newitem->prev=c->prev;if (newitem->next) newitem->next->prev=newitem;
		if (!fail)	out=(char*)cJSON_malloc(len);
	if (c==array->child) array->child=newitem; else newitem->prev->next=newitem;c->next=c->prev=0;cJSON_Delete(c);}
		/* If that fails, we fail. */
void   cJSON_ReplaceItemInObject(cJSON *object,const char *string,cJSON *newitem){int i=0;cJSON *c=object->child;while(c && cJSON_strcasecmp(c->string,string))i++,c=c->next;if(c){newitem->string=cJSON_strdup(string);cJSON_ReplaceItemInArray(object,i,newitem);}}
		if (!out) fail=1;


/* Create basic types: */
		/* Handle failure. */
cJSON *cJSON_CreateNull(void)					{cJSON *item=cJSON_New_Item();if(item)item->type=cJSON_NULL;return item;}
		if (fail)
cJSON *cJSON_CreateTrue(void)					{cJSON *item=cJSON_New_Item();if(item)item->type=cJSON_True;return item;}
		{
cJSON *cJSON_CreateFalse(void)					{cJSON *item=cJSON_New_Item();if(item)item->type=cJSON_False;return item;}
			for (i=0;i<numentries;i++) if (entries[i]) cJSON_free(entries[i]);
cJSON *cJSON_CreateBool(int b)					{cJSON *item=cJSON_New_Item();if(item)item->type=b?cJSON_True:cJSON_False;return item;}
			cJSON_free(entries);
cJSON *cJSON_CreateNumber(double num)			{cJSON *item=cJSON_New_Item();if(item){item->type=cJSON_Number;item->valuedouble=num;item->valueint=(int)num;}return item;}
			return 0;
cJSON *cJSON_CreateString(const char *string)	{cJSON *item=cJSON_New_Item();if(item){item->type=cJSON_String;item->valuestring=cJSON_strdup(string);}return item;}
		}
cJSON *cJSON_CreateArray(void)					{cJSON *item=cJSON_New_Item();if(item)item->type=cJSON_Array;return item;}
		
cJSON *cJSON_CreateObject(void)					{cJSON *item=cJSON_New_Item();if(item)item->type=cJSON_Object;return item;}
		/* Compose the output array. */

		*out='[';
/* Create Arrays: */
		ptr=out+1;*ptr=0;
cJSON *cJSON_CreateIntArray(const int *numbers,int count)		{int i;cJSON *n=0,*p=0,*a=cJSON_CreateArray();for(i=0;a && i<count;i++){n=cJSON_CreateNumber(numbers[i]);if(!i)a->child=n;else suffix_object(p,n);p=n;}return a;}
		for (i=0;i<numentries;i++)
cJSON *cJSON_CreateFloatArray(const float *numbers,int count)	{int i;cJSON *n=0,*p=0,*a=cJSON_CreateArray();for(i=0;a && i<count;i++){n=cJSON_CreateNumber(numbers[i]);if(!i)a->child=n;else suffix_object(p,n);p=n;}return a;}
		{
cJSON *cJSON_CreateDoubleArray(const double *numbers,int count)	{int i;cJSON *n=0,*p=0,*a=cJSON_CreateArray();for(i=0;a && i<count;i++){n=cJSON_CreateNumber(numbers[i]);if(!i)a->child=n;else suffix_object(p,n);p=n;}return a;}
			tmplen=strlen(entries[i]);memcpy(ptr,entries[i],tmplen);ptr+=tmplen;
cJSON *cJSON_CreateStringArray(const char **strings,int count)	{int i;cJSON *n=0,*p=0,*a=cJSON_CreateArray();for(i=0;a && i<count;i++){n=cJSON_CreateString(strings[i]);if(!i)a->child=n;else suffix_object(p,n);p=n;}return a;}
			if (i!=numentries-1) {*ptr++=',';if(fmt)*ptr++=' ';*ptr=0;}

			cJSON_free(entries[i]);
/* Duplication */
		}
cJSON *cJSON_Duplicate(cJSON *item,int recurse)
{
	cJSON *newitem,*cptr,*nptr=0,*newchild;
	/* Bail on bad ptr */
	item->type=cJSON_Object;
static const char *parse_object(cJSON *item,const char *value,const char **ep)
	value=skip(value+1);
	newitem->type=item->type&(~cJSON_IsReference),newitem->valueint=item->valueint,newitem->valuedouble=item->valuedouble;
	if (*value=='}') return value+1;	/* empty array. */
	item->child=child=cJSON_New_Item();
	if (item->valuestring)	{newitem->valuestring=cJSON_strdup(item->valuestring);	if (!newitem->valuestring)	{cJSON_Delete(newitem);return 0;}}
	if (!item->child) return 0;
	if (item->string)		{newitem->string=cJSON_strdup(item->string);			if (!newitem->string)		{cJSON_Delete(newitem);return 0;}}
	value=skip(parse_string(child,skip(value),ep));
	if (!recurse) return newitem;
	if (!value) return 0;
	cptr=item->child;
	child->string=child->valuestring;child->valuestring=0;
	while (cptr)
	if (*value!=':') {*ep=value;return 0;}	/* fail! */
	{
	value=skip(parse_value(child,skip(value+1),ep));	/* skip any spacing, get the value. */
		newchild=cJSON_Duplicate(cptr,1);		/* Duplicate (with recurse) each item in the ->next chain */
		if (nptr)	{nptr->next=newchild,newchild->prev=nptr;nptr=newchild;}	/* If newitem->child already set, then crosswire ->prev and ->next and move on */
		else		{newitem->child=newchild;nptr=newchild;}					/* Set newitem->child and move to it */
		cptr=cptr->next;
	}
void cJSON_Minify(char *json)
	{
{
	char *into=json;
		if (!(new_item=cJSON_New_Item()))	return 0; /* memory fail */
	while (*json)
		child->next=new_item;new_item->prev=child;child=new_item;
	{
		value=skip(parse_string(child,skip(value+1),ep));
		if (*json==' ') json++;
		if (!value) return 0;
		else if (*json=='\t') json++;	// Whitespace characters.
		child->string=child->valuestring;child->valuestring=0;
		else if (*json=='\r') json++;
		if (*value!=':') {*ep=value;return 0;}	/* fail! */
		else if (*json=='\n') json++;
		value=skip(parse_value(child,skip(value+1),ep));	/* skip any spacing, get the value. */
		else if (*json=='/' && json[1]=='/')  while (*json && *json!='\n') json++;	// double-slash comments, to end of line.
		else if (*json=='/' && json[1]=='*') {while (*json && !(*json=='*' && json[1]=='/')) json++;json+=2;}	// multiline comments.
		else if (*json=='\"'){*into++=*json++;while (*json && *json!='\"'){if (*json=='\\') *into++=*json++;*into++=*json++;}*into++=*json++;} // string literals, which are \" sensitive.
		else *into++=*json++;			// All other characters.
	}
	*into=0;	// and null-terminate.
	if (*value=='}') return value+1;	/* end of array */
}
	*ep=value;return 0;	/* malformed. */
static char *print_object(cJSON *item,int depth,int fmt,printbuffer *p)
	char **entries=0,**names=0;
	char *out=0,*ptr,*ret,*str;int len=7,i=0,j;
	cJSON *child=item->child;
	int numentries=0,fail=0;
	size_t tmplen=0;
	while (child) numentries++,child=child->next;
	/* Explicitly handle empty object case */
	if (!numentries)
	{
		if (p) out=ensure(p,fmt?depth+4:3);
		else	out=(char*)cJSON_malloc(fmt?depth+4:3);
		if (!out)	return 0;
		ptr=out;*ptr++='{';
		if (fmt) {*ptr++='\n';for (i=0;i<depth;i++) *ptr++='\t';}
		*ptr++='}';*ptr++=0;
		return out;
	}
	if (p)
	{
		/* Compose the output: */
		i=p->offset;
		len=fmt?2:1;	ptr=ensure(p,len+1);	if (!ptr) return 0;
		*ptr++='{';	if (fmt) *ptr++='\n';	*ptr=0;	p->offset+=len;
		child=item->child;depth++;
		while (child)
		{
			if (fmt)
			{
				ptr=ensure(p,depth);	if (!ptr) return 0;
				for (j=0;j<depth;j++) *ptr++='\t';
				p->offset+=depth;
			}
			print_string_ptr(child->string,p);
			p->offset=update(p);
			
			len=fmt?2:1;
			ptr=ensure(p,len);	if (!ptr) return 0;
			*ptr++=':';if (fmt) *ptr++='\t';
			p->offset+=len;
			
			print_value(child,depth,fmt,p);
			p->offset=update(p);

			len=(fmt?1:0)+(child->next?1:0);
			ptr=ensure(p,len+1); if (!ptr) return 0;
			if (child->next) *ptr++=',';
			if (fmt) *ptr++='\n';*ptr=0;
			p->offset+=len;
			child=child->next;
		ptr=ensure(p,fmt?(depth+1):2);	 if (!ptr) return 0;
		if (fmt)	for (i=0;i<depth-1;i++) *ptr++='\t';
		*ptr++='}';*ptr=0;
		out=(p->buffer)+i;
	else
	{
		/* Allocate space for the names and the objects */
		entries=(char**)cJSON_malloc(numentries*sizeof(char*));
		if (!entries) return 0;
		names=(char**)cJSON_malloc(numentries*sizeof(char*));
		if (!names) {cJSON_free(entries);return 0;}
		memset(entries,0,sizeof(char*)*numentries);
		memset(names,0,sizeof(char*)*numentries);

		/* Collect all the results into our arrays: */
		child=item->child;depth++;if (fmt) len+=depth;
		while (child && !fail)
		{
			names[i]=str=print_string_ptr(child->string,0);
			entries[i++]=ret=print_value(child,depth,fmt,0);
			if (str && ret) len+=strlen(ret)+strlen(str)+2+(fmt?2+depth:0); else fail=1;
			child=child->next;
		}
		
		/* Try to allocate the output string */
		if (!fail)	out=(char*)cJSON_malloc(len);
		if (!out) fail=1;

		/* Handle failure */
		if (fail)
		{
			for (i=0;i<numentries;i++) {if (names[i]) cJSON_free(names[i]);if (entries[i]) cJSON_free(entries[i]);}
			cJSON_free(names);cJSON_free(entries);
			return 0;
		}
		
		/* Compose the output: */
		*out='{';ptr=out+1;if (fmt)*ptr++='\n';*ptr=0;
		for (i=0;i<numentries;i++)
		{
			if (fmt) for (j=0;j<depth;j++) *ptr++='\t';
			tmplen=strlen(names[i]);memcpy(ptr,names[i],tmplen);ptr+=tmplen;
			*ptr++=':';if (fmt) *ptr++='\t';
			strcpy(ptr,entries[i]);ptr+=strlen(entries[i]);
			if (i!=numentries-1) *ptr++=',';
			if (fmt) *ptr++='\n';*ptr=0;
			cJSON_free(names[i]);cJSON_free(entries[i]);
		}
		
		cJSON_free(names);cJSON_free(entries);
		if (fmt) for (i=0;i<depth-1;i++) *ptr++='\t';
		*ptr++='}';*ptr++=0;
/* Get Array size/item / object item. */
int    cJSON_GetArraySize(cJSON *array)							{cJSON *c=array->child;int i=0;while(c)i++,c=c->next;return i;}
cJSON *cJSON_GetArrayItem(cJSON *array,int item)				{cJSON *c=array?array->child:0;while (c && item>0) item--,c=c->next; return c;}
cJSON *cJSON_GetObjectItem(cJSON *object,const char *string)	{cJSON *c=object?object->child:0;while (c && cJSON_strcasecmp(c->string,string)) c=c->next; return c;}
int cJSON_HasObjectItem(cJSON *object,const char *string)		{return cJSON_GetObjectItem(object,string)?1:0;}
static void suffix_object(cJSON *prev,cJSON *item) {prev->next=item;item->prev=prev;}
static cJSON *create_reference(cJSON *item) {cJSON *ref=cJSON_New_Item();if (!ref) return 0;memcpy(ref,item,sizeof(cJSON));ref->string=0;ref->type|=cJSON_IsReference;ref->next=ref->prev=0;return ref;}
void   cJSON_AddItemToArray(cJSON *array, cJSON *item)						{cJSON *c=array->child;if (!item) return; if (!c) {array->child=item;} else {while (c && c->next) c=c->next; suffix_object(c,item);}}
void   cJSON_AddItemToObject(cJSON *object,const char *string,cJSON *item)	{if (!item) return; if (item->string) cJSON_free(item->string);item->string=cJSON_strdup(string);cJSON_AddItemToArray(object,item);}
void   cJSON_AddItemToObjectCS(cJSON *object,const char *string,cJSON *item)	{if (!item) return; if (!(item->type&cJSON_StringIsConst) && item->string) cJSON_free(item->string);item->string=(char*)string;item->type|=cJSON_StringIsConst;cJSON_AddItemToArray(object,item);}
void	cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item)						{cJSON_AddItemToArray(array,create_reference(item));}
void	cJSON_AddItemReferenceToObject(cJSON *object,const char *string,cJSON *item)	{cJSON_AddItemToObject(object,string,create_reference(item));}

cJSON *cJSON_DetachItemFromArray(cJSON *array,int which)			{cJSON *c=array->child;while (c && which>0) c=c->next,which--;if (!c) return 0;
	if (c->prev) c->prev->next=c->next;if (c->next) c->next->prev=c->prev;if (c==array->child) array->child=c->next;c->prev=c->next=0;return c;}
void   cJSON_DeleteItemFromArray(cJSON *array,int which)			{cJSON_Delete(cJSON_DetachItemFromArray(array,which));}
cJSON *cJSON_DetachItemFromObject(cJSON *object,const char *string) {int i=0;cJSON *c=object->child;while (c && cJSON_strcasecmp(c->string,string)) i++,c=c->next;if (c) return cJSON_DetachItemFromArray(object,i);return 0;}
void   cJSON_DeleteItemFromObject(cJSON *object,const char *string) {cJSON_Delete(cJSON_DetachItemFromObject(object,string));}
void   cJSON_InsertItemInArray(cJSON *array,int which,cJSON *newitem)		{cJSON *c=array->child;while (c && which>0) c=c->next,which--;if (!c) {cJSON_AddItemToArray(array,newitem);return;}
	newitem->next=c;newitem->prev=c->prev;c->prev=newitem;if (c==array->child) array->child=newitem; else newitem->prev->next=newitem;}
void   cJSON_ReplaceItemInArray(cJSON *array,int which,cJSON *newitem)		{cJSON *c=array->child;while (c && which>0) c=c->next,which--;if (!c) return;
	newitem->next=c->next;newitem->prev=c->prev;if (newitem->next) newitem->next->prev=newitem;
	if (c==array->child) array->child=newitem; else newitem->prev->next=newitem;c->next=c->prev=0;cJSON_Delete(c);}
void   cJSON_ReplaceItemInObject(cJSON *object,const char *string,cJSON *newitem){int i=0;cJSON *c=object->child;while(c && cJSON_strcasecmp(c->string,string))i++,c=c->next;if(c){newitem->string=cJSON_strdup(string);cJSON_ReplaceItemInArray(object,i,newitem);}}
cJSON *cJSON_CreateNull(void)					{cJSON *item=cJSON_New_Item();if(item)item->type=cJSON_NULL;return item;}
cJSON *cJSON_CreateTrue(void)					{cJSON *item=cJSON_New_Item();if(item)item->type=cJSON_True;return item;}
cJSON *cJSON_CreateFalse(void)					{cJSON *item=cJSON_New_Item();if(item)item->type=cJSON_False;return item;}
cJSON *cJSON_CreateBool(int b)					{cJSON *item=cJSON_New_Item();if(item)item->type=b?cJSON_True:cJSON_False;return item;}
cJSON *cJSON_CreateNumber(double num)			{cJSON *item=cJSON_New_Item();if(item){item->type=cJSON_Number;item->valuedouble=num;item->valueint=(int64_t)num;}return item;}
cJSON *cJSON_CreateString(const char *string)	{cJSON *item=cJSON_New_Item();if(item){item->type=cJSON_String;item->valuestring=cJSON_strdup(string);if(!item->valuestring){cJSON_Delete(item);return 0;}}return item;}
cJSON *cJSON_CreateArray(void)					{cJSON *item=cJSON_New_Item();if(item)item->type=cJSON_Array;return item;}
cJSON *cJSON_CreateObject(void)					{cJSON *item=cJSON_New_Item();if(item)item->type=cJSON_Object;return item;}

/* Create Arrays: */
cJSON *cJSON_CreateIntArray(const int *numbers,int count)		{int i;cJSON *n=0,*p=0,*a=cJSON_CreateArray();for(i=0;a && i<count;i++){n=cJSON_CreateNumber(numbers[i]);if(!n){cJSON_Delete(a);return 0;}if(!i)a->child=n;else suffix_object(p,n);p=n;}return a;}
cJSON *cJSON_CreateFloatArray(const float *numbers,int count)	{int i;cJSON *n=0,*p=0,*a=cJSON_CreateArray();for(i=0;a && i<count;i++){n=cJSON_CreateNumber(numbers[i]);if(!n){cJSON_Delete(a);return 0;}if(!i)a->child=n;else suffix_object(p,n);p=n;}return a;}
cJSON *cJSON_CreateDoubleArray(const double *numbers,int count)	{int i;cJSON *n=0,*p=0,*a=cJSON_CreateArray();for(i=0;a && i<count;i++){n=cJSON_CreateNumber(numbers[i]);if(!n){cJSON_Delete(a);return 0;}if(!i)a->child=n;else suffix_object(p,n);p=n;}return a;}
cJSON *cJSON_CreateStringArray(const char **strings,int count)	{int i;cJSON *n=0,*p=0,*a=cJSON_CreateArray();for(i=0;a && i<count;i++){n=cJSON_CreateString(strings[i]);if(!n){cJSON_Delete(a);return 0;}if(!i)a->child=n;else suffix_object(p,n);p=n;}return a;}

/* Duplication */
cJSON *cJSON_Duplicate(cJSON *item,int recurse)
{
	cJSON *newitem,*cptr,*nptr=0,*newchild;
	/* Bail on bad ptr */
	if (!item) return 0;
	/* Create new item */
	newitem=cJSON_New_Item();
	if (!newitem) return 0;
	/* Copy over all vars */
	newitem->type=item->type&(~cJSON_IsReference),newitem->valueint=item->valueint,newitem->valuedouble=item->valuedouble;
	if (item->valuestring)	{newitem->valuestring=cJSON_strdup(item->valuestring);	if (!newitem->valuestring)	{cJSON_Delete(newitem);return 0;}}
	if (item->string)		{newitem->string=cJSON_strdup(item->string);			if (!newitem->string)		{cJSON_Delete(newitem);return 0;}}
	/* If non-recursive, then we're done! */
	if (!recurse) return newitem;
	/* Walk the ->next chain for the child. */
	cptr=item->child;
	while (cptr)
	{
		newchild=cJSON_Duplicate(cptr,1);		/* Duplicate (with recurse) each item in the ->next chain */
		if (!newchild) {cJSON_Delete(newitem);return 0;}
		if (nptr)	{nptr->next=newchild,newchild->prev=nptr;nptr=newchild;}	/* If newitem->child already set, then crosswire ->prev and ->next and move on */
		else		{newitem->child=newchild;nptr=newchild;}					/* Set newitem->child and move to it */
		cptr=cptr->next;
	}
	return newitem;
}

void cJSON_Minify(char *json)
{
	char *into=json;
	while (*json)
	{
		if (*json==' ') json++;
		else if (*json=='\t') json++;	/* Whitespace characters. */
		else if (*json=='\r') json++;
		else if (*json=='\n') json++;
		else if (*json=='/' && json[1]=='/')  while (*json && *json!='\n') json++;	/* double-slash comments, to end of line. */
		else if (*json=='/' && json[1]=='*') {while (*json && !(*json=='*' && json[1]=='/')) json++;json+=2;}	/* multiline comments. */
		else if (*json=='\"'){*into++=*json++;while (*json && *json!='\"'){if (*json=='\\') *into++=*json++;*into++=*json++;}*into++=*json++;} /* string literals, which are \" sensitive. */
		else *into++=*json++;			/* All other characters. */
	}
	*into=0;	/* and null-terminate. */


#ifndef __WILDCARD_H__
#define __WILDCARD_H__
/*
 * This is the wildcard matching routine. It returns 1 for a
 * successful match, 0 for an unsuccessful match, and <0 for a
 * syntax error in the wildcard.
 */
int wc_match(const char *wildcard, const char *target);

#endif //__WILDCARD_H__

#ifndef CHALL2_H
#define CHALL2_H

#include <stdint.h>

#define NAME "find-pair"
#define USAGE\
     "Usage: %s [OPTION] FILE BALANCE\n\
Find optimal gifts in FILE for 2 or 3 friends less than or equal to BALANCE\n\
\n\
With no OPTION, assumes two friends.\n\
\n\
\t-nFRIEND_COUNT  \twhere FRIEND_COUNT is either 2 or 3\n\
\t-h              \tdisplay this help and exit\n"
#define usage_msg(stream) fprintf(stream, USAGE, NAME)


void parse_args(int argc, char **argv, int *num_friends,
                char **fin_path, uint64_t *giftamt);
int parse_itemtab(struct itemtab *itemtab, FILE *fin);
void two_friends(const struct itemtab *itemtab, uint64_t targ);
void three_friends(const struct itemtab *itemtab, uint64_t targ);
void display_results(FILE *stream, int elt_count,
                     const uint64_t *prices, const char **names);
static struct size_pair max_pair_under(uint64_t ceil, const uint64_t *nums,
                                       size_t from, size_t to);

#endif

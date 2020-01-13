#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "chall2_lib.h"
#include "chall2.h"


/* Parses cmdline arguments. Parses input file of prices, storing the contents
 * in an item table. Calls two_friends or three_friends to determine the optimum
 * combination of gifts. */
int main(int argc, char **argv)
{
     int num_friends;
     char *fin_path;
     uint64_t giftamt;
     parse_args(argc, argv, &num_friends, &fin_path, &giftamt);

     /* initialize input file and item table */
     FILE *fin = fopen(fin_path, "r");
     struct itemtab itemtab = {
          .prices = dyn_buff_init_i(),
          .names = dyn_buff_init_s()
     };

     /* if file doesn't conform to spec, error and indicate the line number */
     int err = parse_itemtab(&itemtab, fin);
     fclose(fin);
     if (err) {
          fprintf(stderr, "File provided is not in proper format.\n");
          fprintf(stderr, "=== Error occured on line %d\n.", err);
          db_free(itemtab.prices);
          db_free(itemtab.names);
          exit(EXIT_FAILURE);
     }

     /* separate algorithms because 2 is linear and 3 is quadratic */
     if (num_friends == 2)
          two_friends(&itemtab, giftamt);
     else if (num_friends == 3)
          three_friends(&itemtab, giftamt);

     db_free_i(itemtab.prices);
     db_free_s(itemtab.names);
     return EXIT_SUCCESS;
}

/* parse_args: handles basic parsing and validation of arguments. */
void parse_args(int argc, char **argv, int *num_friends,
                char **fin_path, uint64_t *giftamt)
{
     opterr = 0;                /* turn off option warnings */
     *num_friends = 2;          /* default */
     char opt;
     char *invalid_chunk;

     if (argc != 3 && argc != 5) {
          usage_msg(stderr);
          exit(EXIT_FAILURE);
     }

     /* parse file path and git card amount */
     *fin_path = argv[argc - 2];
     *giftamt = strtoll(argv[argc - 1], &invalid_chunk, 10);
     /* check for error in conversion, bad pre/suffix, or presence of neg */
     if (errno || strlen(invalid_chunk) > 0 || strchr(argv[argc - 1], '-')) {
          fprintf(stderr, "Giftcard amount is malformed.\n");
          exit(EXIT_FAILURE);
     }

     /* check if file is accessible */
     if (faccessat(AT_FDCWD, *fin_path, F_OK | R_OK, 0)) {
          fprintf(stderr, "Cannot access file at path %s\n", *fin_path);
          exit(EXIT_FAILURE);
     }

     while ((opt = getopt(argc, argv, "n:h")) != -1) {
          switch (opt) {
          case 'n':
               if (strcmp(optarg, "2") == 0) {
                    *num_friends = 2;
               } else if (strcmp(optarg, "3") == 0) {
                    *num_friends = 3;
               } else {
                    fprintf(stderr,
                            "Option %c only takes arguments '2' or '3'", opt);
                    exit(EXIT_FAILURE);
               }
               break;
          case 'h':
               usage_msg(stdout);
               exit(EXIT_SUCCESS);
               break;
          case '?':
               if (optopt == '2' || optopt == '3')
                    fprintf(stderr, "%c should follow the -n option\n", optopt);
               else
                    fprintf(stderr, "Unknown option %c\n", optopt);
               exit(EXIT_FAILURE);
          default:
               usage_msg(stderr);
               exit(EXIT_FAILURE);
          }
     }
}

/* parse_itemtab: Parses contents of FIN into ITEMTAB which must be already
 * initialized. If there are no errors parsing the file, returns 0. Otherwise,
 * returns the line number of FIN where the error occurred.
 */
int parse_itemtab(struct itemtab *itemtab, FILE *fin)
{
     /* char *line, *itemname, *itemprice; */
     char *line = NULL;
     size_t len = 0;
     char *delim = ",";
     char *invalid_chunk;
     char *name, *price;
     uint64_t currprice, lastprice = UINT64_MAX;
     int linum = 0;
     while (getline(&line, &len, fin) != -1) {
          ++linum;
          name = strtok(line, delim);
          price = strtok(NULL, delim);

          /* skips over blank lines */
          if (strcmp(name, "\n") == 0 && !price)
               continue;

          if (!name || !price)
               goto fin;

          currprice = strtoull(price, &invalid_chunk, 10);

          /* permit leading/trailing whitespace and nothing else */
          for (size_t i = 0; i < strlen(invalid_chunk); ++i)
               if (!isspace(invalid_chunk[i]))
                    goto fin;

          if (errno)
               goto fin;

          /* validation. Only 2 cols and sorted ascending positive integers */
          if (strtok(NULL, delim) != NULL || name == NULL || price == NULL
              || (lastprice != UINT64_MAX && lastprice > currprice)
              || strchr(price, '-') != NULL) 
               goto fin;

          /* add name and price to itemtab */
          db_push(itemtab->names, name);
          db_push(itemtab->prices, lastprice = currprice);
     }
     linum = 0;                 /* parsing was successful */
fin:
     free(line);
     return linum;
}

/* two_friends: Linear time since it just depends on max_pair_under.
 */
void two_friends(const struct itemtab *itemtab, uint64_t targ)
{
     /* aliased for readability */
     uint64_t *prices = itemtab->prices->membs;
     char **names = itemtab->names->membs;

     /* get best price pair from 0 to nmemb */
     struct size_pair best = max_pair_under(targ, itemtab->prices->membs,
                                            0, itemtab->prices->nmemb);

     if (best.car != SIZE_MAX && best.cdr != SIZE_MAX) {
          uint64_t bestprices[] = {
               prices[best.car],
               prices[best.cdr]
          };
          const char *bestnames[] = {
               names[best.car],
               names[best.cdr]
          };
          display_results(stdout, 2, bestprices, bestnames);
     } else {
          display_results(stdout, 2, NULL, NULL);
     }
}

/* three_friends: double pointer sweep over a window each time the window
 * narrows. Quadratic.
 */
void three_friends(const struct itemtab *itemtab, uint64_t targ)
{
     uint64_t *prices = itemtab->prices->membs;
     char **names = itemtab->names->membs;
     size_t n = itemtab->prices->nmemb;

     uint64_t curr_max, sum;
     curr_max = sum = 0;
     size_t bestl, bestmid, bestr;
     struct size_pair best;

     size_t i;
     for (i = 0; i < n; ++i) {
          /* get local optimum for sub array in front of i */
          best = max_pair_under(targ - prices[i], prices, i + 1, n);

          if (best.car == SIZE_MAX && best.cdr == SIZE_MAX)
               continue;

          sum = prices[i] + prices[best.car] + prices[best.cdr];

          if (sum > curr_max && sum <= targ) {
               curr_max = sum;
               bestl = i;
               bestmid = best.car;
               bestr = best.cdr;
          }
          if (sum == targ)
               break;
     }

     if (curr_max > 0) {
          uint64_t bestprices[] = {
               prices[bestl],
               prices[bestmid],
               prices[bestr]
          };
          const char *bestnames[] = {
               names[bestl],
               names[bestmid],
               names[bestr]
          };
          display_results(stdout, 3, bestprices, bestnames);
     } else {
          display_results(stdout, 3, NULL, NULL);
     }
}

/* display_results: displays results to STREAM */
void display_results(FILE *stream, int elt_count,
                     const uint64_t *prices, const char **names)
{
     if (prices == NULL && names == NULL) {
          fprintf(stream, "Not possible\n");
     } else {
          int i;
          for (i = 0; i < elt_count - 1; ++i)
               fprintf(stream, "%s %" PRIu64 ", ", names[i], prices[i]);
          fprintf(stream, "%s %" PRIu64 "\n", names[i], prices[i]);
     }
}

/* max_pair_under: finds pair of numbers in NUMS[FROM:TO] with maximal sum less
 * than or equal to CEIL. Returns the indexes of the pairs in nums. If no pair
 * is found, both members of the pair will be UINT64_MAX. Linear time */
/* static struct size_pair max_pair_under(uint64_t ceil, const elt *nums, */
                                       /* size_t from, size_t to) */
static struct size_pair max_pair_under(uint64_t ceil, const uint64_t *nums,
                                       size_t from, size_t to)
{
     uint64_t sum, curr_max;
     curr_max = 0;              /* current best sum */
     size_t bestl, bestr, l, r;
     bestl = bestr = SIZE_MAX;  /* default if no pair found */
     l = from;                  /* left most */
     r = to - 1;                /* and right most indices */

     while (l < r) {
          /* Fail if sum isn't representable with 64 bits */
          assert(UINT64_MAX > nums[l] + nums[r]);

          sum = nums[l] + nums[r];

          if (sum > ceil) {
               --r;
               continue;
          }
          if (sum > curr_max && sum <= ceil) {
               curr_max = sum;
               bestl = l; bestr = r;
          }
          if (sum == ceil)
               break;
          ++l;
     }

     struct size_pair ret = {
          .car = bestl,
          .cdr = bestr
     };

     return ret;
}

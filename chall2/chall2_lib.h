#ifndef CHALL2_LIB_H
#define CHALL2_LIB_H

#include <stdint.h>
#include <tgmath.h>

/*
 * Provides a simple implementation of dynamically resizing buffers. The buffers
 * resize by a factor of two each time, making the number of times we have to
 * allocate mem to them logarithmic. dyn_buff_i and dyn_buff_s define the types
 * of dynamic buffers, storing a uint64_t integer or a string,
 * respectively. Members can be accessed and added in constant time.
 *
 * When pushing to or freeing dynamic buffers, use the generic macros db_push
 * and db_free.
 */


#define db_push(place, newelt) _Generic((place),                        \
                                        struct dyn_buff_i *: db_push_i, \
                                        struct dyn_buff_s *: db_push_s  \
          )(place, newelt)

#define db_free(db) _Generic((db),                              \
                             struct dyn_buff_i *: db_free_i,    \
                             struct dyn_buff_s *: db_free_s     \
          )(db)

struct dyn_buff_i {
     size_t nmemb;
     size_t memb_limit;
     uint64_t *membs;
};

struct dyn_buff_s {
     size_t nmemb;
     size_t memb_limit;
     char **membs;
};

/* simple item table to relate prices to names */
struct itemtab {
     struct dyn_buff_i *prices;
     struct dyn_buff_s *names;
};

/* basic pair type for array indices to make calling max_pair_under less ugly */
struct size_pair {
     size_t car;
     size_t cdr;
};

struct dyn_buff_i *dyn_buff_init_i();
struct dyn_buff_s *dyn_buff_init_s();
void db_free_s(struct dyn_buff_s *db);
void db_free_i(struct dyn_buff_i *db);
void db_push_i(struct dyn_buff_i *place, uint64_t newelt);
void db_push_s(struct dyn_buff_s *place, const char *newelt);

#endif

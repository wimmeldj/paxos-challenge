#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "chall2_lib.h"

/* dyn_buff_i: alloc and return a dynamic buffer storing uint64_t members */
struct dyn_buff_i *dyn_buff_init_i()
{
     struct dyn_buff_i *db = calloc(1, sizeof(*db));
     db->nmemb = 0;
     db->memb_limit = 10;
     db->membs = calloc(10, sizeof(*db->membs));
     return db;
}

/* dyn_buff_s: alloc and return a dynamic buffer storing string members */
struct dyn_buff_s *dyn_buff_init_s()
{
     struct dyn_buff_s *db = calloc(1, sizeof(*db));
     db->nmemb = 0;
     db->memb_limit = 10;
     db->membs = calloc(10, sizeof(*db->membs));
     return db;
}

/* basic memory freeing functions for dynamic buffers of type string and
 * uint64_t */
void db_free_i(struct dyn_buff_i *db)
{
     free(db->membs);
     free(db);
}
void db_free_s(struct dyn_buff_s *db)
{
     size_t i = 0;
     while (i < db->nmemb)
          free(db->membs[i++]);
     free(db->membs);
     free(db);
}

/* db_push_s: pushes string, NEWELT, onto PLACE's membs array, resizing if
 * necessary */
void db_push_s(struct dyn_buff_s *place, const char *newelt)
{
     const size_t len = strlen(newelt);
     char* copy = calloc(len + 1, 1);
     strncpy(copy, newelt, len);

     if (place->nmemb == place->memb_limit)
          place->membs = reallocarray(place->membs, place->memb_limit *= 2,
                                      sizeof(*place->membs));

     place->membs[place->nmemb++] = copy;
}

/* db_push_s: pushes uint64_t, NEWELT, onto PLACE's membs array, resizing if
 * necessary */
void db_push_i(struct dyn_buff_i *place, uint64_t newelt)
{
     if (place->nmemb == place->memb_limit)
          place->membs = reallocarray(place->membs, place->memb_limit *= 2,
                                      sizeof(*place->membs));
     place->membs[place->nmemb++] = newelt;
}

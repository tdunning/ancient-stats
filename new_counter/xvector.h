/* 
  copyright 1994 ted dunning

  unlimited noncommercial use, copying and redistribution allowed.

  commercial use, copying and redistribution allowed by written
  permission.  contact ted@nmsu.edu for more information.


  extendable vectors.

   an x vector is any structure which has integer fields called size
   and count and a pointer to something called contents.  size is used
   as the current number of elements pointed to by contents and count
   is used to indicate the next free element of contents.

   when contents fills up, it is reallocated with twice as many
   elements.  this leads to log n reallocations as an xvector is grown
   from 1 to n elements.

   an x vector with no fields other than the size, count and contents
   can be declared using  

	x_vector(xvector_type_name, base_type)   

   an x vector should be created using

	x_create(vector_name, initial_size)

   elements can be added to the end of an x vector using

	x_push(vector_name, new_element_name)
	
   the nth element can be accessed using

   	x_nth(vector_name, index)

   the active length of an x vector can be accessed (or set) using

	x_length(v)
	
   the last element of an x vector can be accessed and deleted in one
   step using

	x_pop(v)

   and any element can be deleted using

	x_delete(v, n)

   note that using x_delete can do odd things to the ordering of the
   elements of an x vector.

   the entire x vector can be freed using

	x_free(vector_name)

   the individual elements of an x vector are not freed by calling
   x_free.  if they need to be freed, then this will have to be done
   explicitly.

   implementing x vectors using macros in this way allows type
   checking to be enforced.

   if the macro x_malloc is defined, then x_malloc, x_calloc and x_free will
   be used instead of the usual malloc, calloc and free.  using the
   default versions makes the program somewhat less safe since no
   checking for allocation failures is done.
   */

#ifndef x_malloc
#define x_malloc malloc

#define x_calloc calloc
#endif

#define x_vector(xv, type) typedef struct {int count, size;type *contents;} *xv

#define x_create(v, n) {v=(void *) x_malloc(sizeof(*v));v->size=n;v->count=0; \
v->contents=(void*) calloc(n, sizeof(v->contents[0]));}

#define x_free(v) {free(v->contents);free(v);}

#define x_push(v, value) {x_resize(v, x_length(v)+1);(v)->contents[(v)->count++]=(value);}
#define x_resize(v, n) {while (n > (v)->size){\
  (v)->size = 2*(v)->size;\
  (v)->contents=(void*) realloc((v)->contents,(v)->size * sizeof((v)->contents[0]));\
    }}

#define x_length(v) ((v)->count)
#define x_pop(v) ((v)->contents[--x_length(v)])
#define x_delete(v, n)  {(v)->contents[n] = (v)->contents[--x_length(v)];}

#define x_nth(v, n) ((v)->contents[n])

#define x_extend(v, n) x_resize(v, x_length(v)+n)

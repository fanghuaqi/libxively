// Copyright (c) 2003-2013, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.
#include "tinytest.h"
#include "tinytest_macros.h"

#include "xi_heap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

///////////////////////////////////////////////////////////////////////////////
// HEAP TESTS
///////////////////////////////////////////////////////////////////////////////
void test_heap_index_calculus( void* data )
{
    (void)(data);

    XI_HEAP_INDEX_TYPE index = 0;

    tt_assert( RIGHT( index ) == 2 );
    tt_assert( LEFT( index ) == 1 );
    tt_assert( PARENT( RIGHT( index ) ) == 0 );
    tt_assert( PARENT( LEFT( index ) ) == 0 );

 end:
    ;
}

void test_heap_create(void* data)
{
    (void)(data);

    xi_heap_t* heap = xi_heap_create( 32 );

    tt_assert( heap != 0 );
    tt_assert( heap->capacity == 32 );
    tt_assert( heap->first_free == 0 );

    for( int i = 0; i < heap->capacity; ++i )
    {
        tt_assert( heap->elements[ i ] != 0 );
    }

    /* Every test-case function needs to finish with an "end:"
       label and (optionally) code to clean up local variables. */
 end:
    xi_heap_destroy( heap );
}

void test_heap_add(void* data)
{
    (void)(data);

    xi_heap_t* heap = xi_heap_create( 32 );

    xi_heap_element_add( heap, 12, ( void* ) 12 );

    tt_assert( heap->first_free == 1 );

    tt_assert( ( heap->elements[ 0 ] )->index == 0 );
    tt_assert( ( heap->elements[ 0 ] )->key == 12 );
    tt_assert( ( heap->elements[ 0 ] )->value == ( void* ) 12 );

    xi_heap_element_add( heap, 13, ( void* ) 13 );

    tt_assert( heap->first_free == 2 );

    tt_assert( ( heap->elements[ 1 ] )->index == 1 );
    tt_assert( ( heap->elements[ 1 ] )->key == 13 );
    tt_assert( ( heap->elements[ 1 ] )->value == ( void* ) 13 );

    xi_heap_element_add( heap, 1, ( void* ) 1 );

    tt_assert( ( heap->elements[ 0 ] )->index == 0 );
    tt_assert( ( heap->elements[ 0 ] )->key == 1 );
    tt_assert( ( heap->elements[ 0 ] )->value == ( void* ) 1 );

    tt_assert( ( heap->elements[ 2 ] )->index == 2 );
    tt_assert( ( heap->elements[ 2 ] )->key == 12 );
    tt_assert( ( heap->elements[ 2 ] )->value == ( void* ) 12 );

 end:
    xi_heap_destroy( heap );
}

void test_heap_sequencial_add( void* data)
{
    (void)(data);

    xi_heap_t* heap = xi_heap_create( 32 );

    for( size_t i = 0; i < 16; ++i )
    {
        xi_heap_element_add( heap, i, ( void* ) i );
    }

    for( size_t i = 0; i < 16; ++i )
    {
        tt_assert( heap->elements[ i ]->index == i );
        tt_assert( heap->elements[ i ]->value == ( void* ) i );
        tt_assert( heap->elements[ i ]->key == i );
    }

 end:
    xi_heap_destroy( heap );
}

void test_heap( xi_heap_t* heap, XI_HEAP_INDEX_TYPE index )
{
    if( index >= heap->first_free ) { return; }

    xi_heap_element_t* e    = heap->elements[ index ];

    XI_HEAP_INDEX_TYPE li   = LEFT( index );
    XI_HEAP_INDEX_TYPE ri   = RIGHT( index );

    li = li >= heap->first_free ? index : li;
    ri = ri >= heap->first_free ? index : ri;

    if( li == index || ri == index ) return;

    xi_heap_element_t* le   = heap->elements[ li ];
    xi_heap_element_t* re   = heap->elements[ ri ];

    tt_assert( e->key <= le->key );
    tt_assert( e->key <= re->key );

    test_heap( heap, li );
    test_heap( heap, ri );
 end:
    ;
}

void test_heap_random_add( void* data )
{
    (void)(data);

    xi_heap_t* heap = xi_heap_create( 128 );
    srand( time( 0 ) );

    for( size_t i = 0; i < 64; ++i )
    {
        XI_HEAP_INDEX_TYPE index = rand() & 63;
        xi_heap_element_add( heap, index, ( void* ) i );
    }

    test_heap( heap, 0 );

 end:
    xi_heap_destroy( heap );
}

void test_heap_random_remove( void* data )
{
    (void)(data);

    xi_heap_t* heap = xi_heap_create( 128 );
    srand( time( 0 ) );

    for( size_t i = 0; i < 64; ++i )
    {
        XI_HEAP_INDEX_TYPE index = rand() & 63;
        xi_heap_element_add( heap, index, ( void* ) i );
    }

    XI_HEAP_KEY_TYPE key = 0;

    for( size_t i = 0; i < 64; ++i )
    {
        const xi_heap_element_t* e = xi_heap_get_top( heap );
        tt_assert( key <= e->key );
        key = e->key;
    }

 end:
    xi_heap_destroy( heap );
}

struct testcase_t datastructures_tests[] = {
    /* Here's a really simple test: it has a name you can refer to it
       with, and a function to invoke it. */
    { "test_heap_index_calculus", test_heap_index_calculus, TT_ENABLED_, 0, 0 },
    { "test_heap_create", test_heap_create, TT_ENABLED_, 0, 0 },
    { "test_heap_add", test_heap_add, TT_ENABLED_, 0, 0 },
    { "test_heap_sequencial_add", test_heap_sequencial_add, TT_ENABLED_, 0, 0 },
    { "test_heap_random_add", test_heap_random_add, TT_ENABLED_, 0, 0 },
    { "test_heap_random_remove", test_heap_random_remove, TT_ENABLED_, 0, 0 },
    /* The array has to end with END_OF_TESTCASES. */
    END_OF_TESTCASES
};

/* Next, we make an array of testgroups.  This is mandatory.  Unlike more
   heavy-duty testing frameworks, groups can't nest. */
struct testgroup_t groups[] = {

    /* Every group has a 'prefix', and an array of tests.  That's it. */
    { "heap/", datastructures_tests },

    END_OF_GROUPS
};

#if XI_UNIT_TEST_NATIVE
int main( int argc, char const *argv[] )
{
  int r = tinytest_main( argc, argv, groups );
  printf( "status: %i\n", r );
  return r;
}
#else
int main()
{
  const char a[] = {"sim"};
  int r = tinytest_main( 1, (const char **) a, groups );
  printf( "status: %i\n", r);
  return r;
}
#endif

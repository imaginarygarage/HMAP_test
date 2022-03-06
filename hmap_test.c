/*********************************************************************************
 *
 *  FILENAME:
 *      hmap_test.c
 *  
 *  DESCRIPTION:
 *      Test the hash mapping module.
 *
 ********************************************************************************/


/*--------------------------------------------------------------------------------
                                     INCLUDES
--------------------------------------------------------------------------------*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hmap/hmap_intf.h"


/*--------------------------------------------------------------------------------
                             PREPROCESSOR DEFINITIONS
--------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------
                                      TYPES
--------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------
                                 MEMORY CONSTANTS
--------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------
                                 STATIC VARIABLES
--------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------
                                    PROCEDURES
--------------------------------------------------------------------------------*/

static unsigned int find_collision
    (
    HMAP_obj_type     * hmap,       /* hash map object                  */
    HMAP_hash_val_type  hash,       /* hash value to match              */
    HMAP_anon_type    * collision,  /* collision data to iterate on     */
    unsigned int        prefix_len, /* size of prefix to be preserved   */
    unsigned int        max_len     /* maxmimum size of colliding data  */
    );

static void log_test_msg
    (
    const char        * frmt_str,   /* format string                    */
                        ...         /* argument list                    */
    );

/*************************************************************************
 *
 *  Procedure:
 *      main
 *
 *  Description:
 *      Main entry point for the HMAP test.
 *
 ************************************************************************/
int main
    (
    void
    )
{
/*-------------------------------------------------------------
Local preprocessor definitions
-------------------------------------------------------------*/
#define EMPTY_MEMORY_1024       ( 8248 )
#define KEY_COLLISION_PREFIX    "key 1 collision "

#define anon_const_data( data ) { data, sizeof( data ) }

/*-------------------------------------------------------------
Local memory constants
-------------------------------------------------------------*/
static const HMAP_anon_type key_1 = anon_const_data( "key 1" );
static const HMAP_anon_type key_2 = anon_const_data( "key 2" );
static const HMAP_anon_type key_3 = anon_const_data( "key 3" );
static const HMAP_anon_type collision = anon_const_data( "key 1 collision bJo5LP" );

static const HMAP_anon_type data_1 = anon_const_data( "key 1 data" );
static const HMAP_anon_type data_2 = anon_const_data( "key 2 data" );
static const HMAP_anon_type data_3 = anon_const_data( "key 3 data" );
static const HMAP_anon_type data_collision = anon_const_data( "collision data" );

/*-------------------------------------------------------------
Local variables
-------------------------------------------------------------*/
char                    collision_found;
HMAP_anon_type          data;
char                    data_buf[ 20 ];
unsigned int            entry_count;
HMAP_obj_type           hmap;
HMAP_def_type           hmap_def;
unsigned int            hmap_size;
HMAP_status_t8          hmap_status;
HMAP_anon_type          key1_collision;
char                    key1_collision_buffer[ 23 ];
HMAP_hash_val_type      key1_collision_hash;
HMAP_hash_val_type      key1_hash;

/*-------------------------------------------------------------
Print test header.
-------------------------------------------------------------*/
log_test_msg
    (
    "---------------\n"
    "HMAP Test Start\n"
    "---------------\n"
    );

/*-------------------------------------------------------------
Define the hash map.
-------------------------------------------------------------*/
hmap_def.map_size = 1024;
hmap_def.hash_type = HMAP_HASH_FUNC_SDBM;
hmap_def.malloc = malloc;
hmap_def.free = free;

data.ptr = data_buf;
data.size = sizeof( data_buf );

/*-------------------------------------------------------------
Create the hash map.
-------------------------------------------------------------*/
log_test_msg( "Creating HMAP of size %d.", hmap_def.map_size );
hmap_status = HMAP_create( &hmap_def, &hmap );
if( hmap_status != HMAP_STATUS_SUCCESS )
    {
    log_test_msg( "HMAP creation failed." );
    return( hmap_status );
    }
log_test_msg( "HMAP created successfully." );

/*-------------------------------------------------------------
Verify the number of entries.
-------------------------------------------------------------*/
HMAP_get_entry_count( &hmap, &entry_count );
if( entry_count != 0 )
    {
    log_test_msg( "Unexpected number of map entries: %i", entry_count );
    return( -1 );
    }
log_test_msg( "HMAP confirmed to be empty" );

/*-------------------------------------------------------------
Verify size of empty map.
-------------------------------------------------------------*/
hmap_status = HMAP_get_size( &hmap, &hmap_size);
if( hmap_status != HMAP_STATUS_SUCCESS )
    {
    log_test_msg( "Failed to determine HMAP memory footprint.");
    return( hmap_status );
    }
else if( hmap_size != EMPTY_MEMORY_1024 )
    {
    log_test_msg
        (
        "Unexpected HMAP memory footprint of size %i bytes. Verify change was intentional and update test.",
        hmap_size 
        );
    return( -1 );
    }
log_test_msg
    (
    "Size of empty %i-bucket HMAP: %i bytes",
    hmap_def.map_size,
    hmap_size 
    );

/*-------------------------------------------------------------
Set the private hash map data.
-------------------------------------------------------------*/
log_test_msg( "Set map data: \"%s\" -> \"%s\"", (char *)key_1.ptr, (char *)data_1.ptr );
HMAP_set_data( &hmap, &key_1, &data_1 );

log_test_msg( "Set map data: \"%s\" -> \"%s\"", (char *)key_2.ptr, (char *)data_2.ptr );
HMAP_set_data( &hmap, &key_2, &data_2 );

log_test_msg( "Set map data: \"%s\" -> \"%s\"", (char *)key_3.ptr, (char *)data_3.ptr );
HMAP_set_data( &hmap, &key_3, &data_3 );

/*-------------------------------------------------------------
Verify the number of entries.
-------------------------------------------------------------*/
HMAP_get_entry_count( &hmap, &entry_count );
if( entry_count != 3 )
    {
    log_test_msg( "Unexpected number of map entries: %i", entry_count );
    return( -1 );
    }
log_test_msg( "Number of map entries confirmed to be %i", entry_count );

/*-------------------------------------------------------------
Verify key collision.
-------------------------------------------------------------*/
HMAP_get_hash( &hmap, &key_1, &key1_hash );
HMAP_get_hash( &hmap, &collision, &key1_collision_hash);
if( key1_hash != key1_collision_hash )
    {
    collision_found = 0;
    log_test_msg( "Finding collision for \"%s\", hash: %i...", (char *)key_1.ptr, key1_hash ); 
    key1_collision.ptr = key1_collision_buffer;
    key1_collision.size = sizeof( KEY_COLLISION_PREFIX );
    memcpy( key1_collision_buffer, KEY_COLLISION_PREFIX, key1_collision.size );
    if( find_collision( &hmap, key1_hash, &key1_collision, key1_collision.size, sizeof( key1_collision_buffer ) ) )
        {
        log_test_msg( "Collision found for \"%s\": \"%s\"", (char *)key_1.ptr, key1_collision_buffer );
        log_test_msg( "Update test's collision key.");
        }
    else
        {
        log_test_msg( "No collision found, increase maximum collision key length." );
        }
    
    return( -1 );
    }
log_test_msg( "\"%s\" verified to be a collision for \"%s\"", (char *)collision.ptr, (char *)key_1.ptr );

/*-------------------------------------------------------------
Add collision entry.
-------------------------------------------------------------*/
log_test_msg( "Set map data: \"%s\" -> \"%s\"", (char *)collision.ptr, (char *)data_collision.ptr );
HMAP_set_data( &hmap, &collision, &data_collision );

/*-------------------------------------------------------------
Verify the number of entries.
-------------------------------------------------------------*/
HMAP_get_entry_count( &hmap, &entry_count );
if( entry_count != 4 )
    {
    log_test_msg( "Unexpected number of map entries: %i", entry_count );
    return( -1 );
    }
log_test_msg( "Number of map entries confirmed to be %i", entry_count );

/*-------------------------------------------------------------
Verify the hash map data.
-------------------------------------------------------------*/
HMAP_get_data( &hmap, &key_1, &data );
log_test_msg( "Verify data: \"%s\" -> \"%s\"", (char *)key_1.ptr, (char *)data.ptr );

HMAP_get_data( &hmap, &key_2, &data );
log_test_msg( "Verify data: \"%s\" -> \"%s\"", (char *)key_2.ptr, (char *)data.ptr );

HMAP_get_data( &hmap, &key_3, &data );
log_test_msg( "Verify data: \"%s\" -> \"%s\"", (char *)key_3.ptr, (char *)data.ptr );

HMAP_get_data( &hmap, &collision, &data );
log_test_msg( "Verify data: \"%s\" -> \"%s\"", (char *)collision.ptr, (char *)data.ptr );

/*-------------------------------------------------------------
Get the entry associated with the key.
-------------------------------------------------------------*/
HMAP_get_entry_count( &hmap, &entry_count );
log_test_msg( "Number of map entries: %d", entry_count );

HMAP_get_size( &hmap, &hmap_size );
log_test_msg( "Total size of map: %d Bytes", hmap_size );

HMAP_remove_entry( &hmap, &key_1 );
log_test_msg( "Remove \"%s\".", (char *)key_1.ptr );

if( HMAP_key_in_map( &hmap, &key_1 ) )
    {
    log_test_msg( "Key removal failed." );
    }
else
    {
    log_test_msg( "Key removed successfully." );
    }

HMAP_get_entry_count( &hmap, &entry_count );
log_test_msg( "Number of map entries: %d", entry_count );

HMAP_get_size( &hmap, &hmap_size );
log_test_msg( "Total size of map: %d Bytes", hmap_size );

HMAP_destroy( &hmap );

return( HMAP_STATUS_SUCCESS );

}   /* main() */


/*************************************************************************
 *
 *  Procedure:
 *      find_collision
 *
 *  Description:
 *      Finds a string that hashes to the same value as the provided hash.
 *      Returns 1 if a collision was found, 0 otherwise.
 *
 ************************************************************************/
static unsigned int find_collision
    (
    HMAP_obj_type     * hmap,       /* hash map object                  */
    HMAP_hash_val_type  hash,       /* hash value to match              */
    HMAP_anon_type    * collision,  /* collision data to iterate on     */
    unsigned int        prefix_len, /* size of prefix to be preserved   */
    unsigned int        max_len     /* maxmimum size of colliding data  */
    )
{
/*-------------------------------------------------------------
Local memory constants
-------------------------------------------------------------*/
static const char       char_set[] = "abcdefghijklmnopqrstuvwxyz"
                                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                     "0123456789";

/*-------------------------------------------------------------
Local variables
-------------------------------------------------------------*/
unsigned int            i;
HMAP_hash_val_type      collision_hash;

/*-------------------------------------------------------------
Depth first recursive search
-------------------------------------------------------------*/
for( i = 0; i < sizeof( char_set ) - 1; i++ )
    {
    collision->size = prefix_len + 1;
    ( (char *)collision->ptr )[ collision->size - 2 ] = char_set[ i ];
    ( (char *)collision->ptr )[ collision->size - 1 ] = '\0';
    HMAP_get_hash( hmap, collision, &collision_hash );
    if( collision_hash == hash )
        {
        return( 1 );
        }
    else if( collision->size < max_len
         && find_collision( hmap, hash, collision, collision->size, max_len ) )
        {
        return ( 1 );
        }
    }

return( 0 );

}   /* find_collision() */


/*************************************************************************
 *
 *  Procedure:
 *      log_test_msg
 *
 *  Description:
 *      Write a message to the test log. Supports formatted strings.
 *
 ************************************************************************/
static void log_test_msg
    (
    const char        * frmt_str,   /* format string                    */
                        ...         /* variable length argument list    */
    )
{
/*-------------------------------------------------------------
Local variables
-------------------------------------------------------------*/
va_list                 args;

/*-------------------------------------------------------------
Print the message to the console.
-------------------------------------------------------------*/
va_start( args, frmt_str );
vprintf( frmt_str, args );
va_end( args );
printf( "\n" );

/*-------------------------------------------------------------
Force the message to be written to the console now.
-------------------------------------------------------------*/
fflush( stdout );

}   /* find_collision() */

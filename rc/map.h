#ifndef MAP_H
#define MAP_H

#include <time.h> // time
#include <stdlib.h> // rand
#include "texture.h"


#define max(x, y) ( x > y ? x : y )
#define min(y, x) ( x < y ? x : y )


typedef unsigned int MAP_type_t;
#define MAP_TYPE_VOID 0
#define MAP_TYPE_WALL 1
#define MAP_TYPE_PASS 2
#define MAP_TYPE_DOOR 3


typedef struct MAP_Map {
  MAP_type_t *array;
  int width;
  int height;
  int length;
} MAP_map_t;


// [ceiling texture:16][floor/wall texture:16][solid:2][type:16]
#define MAP_BTYPE_VOID        0
#define MAP_BTYPE_WALL        1
#define MAP_BTYPE_DOOR        2
#define MAP_BTYPE_OPENDOOR    3
#define MAP_BTYPE_OPENINGDOOR 5
#define MAP_BTYPE_REDDOOR     6
#define MAP_BTYPE_BLUEDOOR    7
#define MAP_BTYPE_GREENDOOR   8
#define MAP_BTYPE_CYANDOOR    9
#define MAP_BSOLID_FALSE      0
#define MAP_BSOLID_TRUE       1



void
MAP_kill ( MAP_map_t *o ) {
  if ( o->array ) free ( o->array );
}


void
MAP_init ( MAP_map_t *o, const int w, const int h ) {
  MAP_kill ( o );
  o->width = w;
  o->height = h;
  o->length = w * h;
  o->array = malloc ( o->length * sizeof o->array );
}


void
MAP_copy ( MAP_map_t *o, const MAP_map_t *m ) {
  MAP_init ( o, m->width, m->height );
  for ( int i = 0; i < m->length; ++i ) o->array[i] = m->array[i];
}


// Fill
void
MAP_fill (
  MAP_map_t *o,
  MAP_type_t v,
  const int x0,
  const int y0,
  const int w0,
  const int h0 )
{
  for ( int y = y0; y < h0 + y0; ++y ) {
    for ( int x = x0; x < w0 + x0; ++x ) {
      o->array [ y * o->width + x ] = v;
    }
  }
}


// Borders
void
MAP_borders ( MAP_map_t *o, MAP_type_t v ) {
  for ( int y = 0; y < o->height; ++y ) {
    o->array [ y * o->width + ( 0            ) ] = v;
    o->array [ y * o->width + ( o->width - 1 ) ] = v;
  }
  for ( int x = 0; x < o->width; ++x ) {
    o->array [ ( 0             ) * o->width + x ] = v;
    o->array [ ( o->height - 1 ) * o->width + x ] = v;
  }
}


// Recoursive division algorithm
#define MAP_RDMAZE_DEPTH     0
#define MAP_RDMAZE_CATACOMBS 0
#define MAP_RDMAZE_ROOMS     1
void
MAP_rdmaze (
  MAP_map_t *o,
  const int x0,
  const int y0,
  const int w0,
  const int h0,
  const int depth,
  const int scaler )
{
  const int si = scaler << 1; // si must be even because of odd doors
  const int s = 5 + 2 * abs ( si );
  if ( w0 < s || h0 < s ) return;
  
  const int px = x0 + 2 + si + ( rand () % max ( 1, w0 - ( 2 + si << 1 ) >> 1 ) << 1 );
  const int py = y0 + 2 + si + ( rand () % max ( 1, h0 - ( 2 + si << 1 ) >> 1 ) << 1 );
  
  for ( int x = x0; x < x0 + w0; ++x ) {
    if ( o->array [ py * o->width + x ] == MAP_TYPE_PASS ) {
      printf ( "MAP_rdmaze: door destroyed at depth %d\n", depth );
      o->array [ ( py - 1 ) * o->width + x ] = MAP_TYPE_PASS;
      o->array [ ( py + 1 ) * o->width + x ] = MAP_TYPE_PASS;
    }
    o->array [ py * o->width + x ] = MAP_TYPE_WALL;
  }
  
  for ( int y = y0; y < y0 + h0; ++y ) {
    if ( o->array [ y * o->width + px ] == MAP_TYPE_PASS ) {
      printf ( "MAP_rdmaze: door destroyed at depth %d\n", depth );
      o->array [ y * o->width + ( px - 1 ) ] = MAP_TYPE_PASS;
      o->array [ y * o->width + ( px + 1 ) ] = MAP_TYPE_PASS;
    }
    o->array [ y * o->width + px ] = MAP_TYPE_WALL;
  }

  const int wl = px - x0 + 1;
  const int pl = ( rand () % ( wl - 1 >> 1 ) << 1 ) + 1;
  
  // HACK: w/o " - scaler " wr/hb is bigger than need to and
  //       door can be placed into a border. need tests for
  //       scaler > 1.
  const int wr = w0 - ( px - x0 ) - scaler;
  const int pr = ( rand () % max ( 1, wr - 1 >> 1 ) << 1 ) + 1;

  const int ht = py - y0 + 1;
  const int pt = ( rand () % ( ht - 1 >> 1 ) << 1 ) + 1;

  const int hb = h0 - ( py - y0 ) - scaler;
  const int pb = ( rand () % max ( 1, hb - 1 >> 1 ) << 1 ) + 1;

  const int skip = rand () % 4;
  if ( skip != 0 ) o->array [ ( py ) * o->width + ( x0 + pl ) ] = MAP_TYPE_PASS;
  if ( skip != 1 ) o->array [ ( py ) * o->width + ( px + pr ) ] = MAP_TYPE_PASS;
  if ( skip != 2 ) o->array [ ( y0 + pt ) * o->width + ( px ) ] = MAP_TYPE_PASS;
  if ( skip != 3 ) o->array [ ( py + pb ) * o->width + ( px ) ] = MAP_TYPE_PASS;

  MAP_rdmaze ( o, x0, y0, wl, ht, depth + 1, scaler );
  MAP_rdmaze ( o, x0, py, wl, hb, depth + 1, scaler );
  MAP_rdmaze ( o, px, y0, wr, ht, depth + 1, scaler );
  MAP_rdmaze ( o, px, py, wr, hb, depth + 1, scaler );
}


#define MAP_CROSS_PASS 0
#define MAP_CROSS_VOID 1
void
MAP_cross ( MAP_map_t *o, const int type ) {
  const int w = o->width >> 1;
  const int h = o->height >> 1;
  
  const int t1 = rand() % 16;
  const int t2 = rand() % 16;
  
  for ( int y = 0; y < o->height; ++y ) {
    o->array [ y * o->width + w ] = MAP_TYPE_WALL;
  }
  
  for ( int x = 0; x < o->width; ++x ) {
    o->array [ h * o->width + x ] = MAP_TYPE_WALL;
  }
  
  // find doors ( cant be only odd, need to be found )
  
  int status = 0;
  
  int dx = rand () % w;
  for ( int i = 0; i < w; ++i ) {
    if ( o->array [ ( h - 1 ) * o->width + ( w - dx ) ] == 0
    &&   o->array [ ( h - 1 ) * o->width + ( w + dx ) ] == 0
    &&   o->array [ ( h + 1 ) * o->width + ( w - dx ) ] == 0
    &&   o->array [ ( h + 1 ) * o->width + ( w + dx ) ] == 0 )
    {
      // found
      status |= 0b01;
      break;
    }
    dx = ( dx + 1 ) % w;
  }
  
  int dy = rand () % h;
  for ( int i = 0; i < h; ++i ) {
    if ( o->array [ ( h - dy ) * o->width + ( w - 1 ) ] == 0
    &&   o->array [ ( h - dy ) * o->width + ( w + 1 ) ] == 0
    &&   o->array [ ( h + dy ) * o->width + ( w - 1 ) ] == 0
    &&   o->array [ ( h + dy ) * o->width + ( w + 1 ) ] == 0 )
    {
      // found
      status |= 0b10;
      break;
    }
    dy = ( dy + 1 ) % h;
  }
  
  if ( type == MAP_CROSS_PASS ) {
    o->array [ h * o->width + ( w - dx ) ] = 2;
    o->array [ h * o->width + ( w + dx ) ] = 2;
    o->array [ ( h - dy ) * o->width + w ] = 2;
    o->array [ ( h + dy ) * o->width + w ] = 2;
      
    // if some door(s) cant be found, then force remove cross
    if ( status != 3 ) {
      printf ( "MAP_cross: door destroyed with status %d\n", status );

      for ( int y = 0; y < o->height; ++y ) {
        o->array [ y * o->width + w ] = 0;
      }
      for ( int x = 0; x < o->width; ++x ) {
        o->array [ h * o->width + x ] = 0;
      }
    }
  }
  else { // if ( type == MAP_CROSS_VOID ) {
    for ( int x = dx; x >= 0; --x ) {
      o->array [ h * o->width + ( w - x ) ] = 0;
      o->array [ h * o->width + ( w + x ) ] = 0;      
    }
    for ( int y = dy; y >= 0; --y ) {
      o->array [ ( h - y ) * o->width + w ] = 0;
      o->array [ ( h + y ) * o->width + w ] = 0;
    }
  } 
}


#define MAP_MIRROR_NO     0b00
#define MAP_MIRROR_PASTE  0b01
#define MAP_MIRROR_MIRROR 0b10
void
MAP_mirror ( MAP_map_t *o, const int mx, const int my ) {
  // const int t1 = rand() % 16;
  // const int t2 = rand() % 16;
  for ( int y = 0; y < o->height >> 1; ++y ) {
    int y0 = y;
    if ( my & MAP_MIRROR_PASTE ) y0 = ( o->height >> 1 ) + y;
    if ( my & MAP_MIRROR_MIRROR ) y0 = o->height - y - 1;
    for ( int x = 0; x < o->width >> 1; ++x ) {
      int x0 = x;
      if ( mx & MAP_MIRROR_PASTE ) x0 = ( o->width >> 1 ) + x;
      if ( mx & MAP_MIRROR_MIRROR ) x0 = o->width - x - 1;
      o->array [ y0 * o->width + x0 ] = o->array [ y * o->width + x ];
      // change textures
      // const int tt1 = ( o->array [ y0 * o->width + x0 ] >>  6 ) & 0xf;
      // const int tt2 = ( o->array [ y0 * o->width + x0 ] >> 10 ) & 0xf;
      // o->array [ y0 * o->width + x0 ] &= 0xf << 10;
      // o->array [ y0 * o->width + x0 ] &= 0xf <<  6;
      // o->array [ y0 * o->width + x0 ] &= 0xf << 10;
      // o->array [ y0 * o->width + x0 ] |= ( t1 * tt1 % 16 ) <<  6;
      // o->array [ y0 * o->width + x0 ] |= ( t2 * tt2 % 16 ) << 10;
    }
  }
}


// Quadrant MAP_rdmaze generator
void
MAP_quadmap ( MAP_map_t *o ) {
  // HACK: " + 0 " creates holes on map where " maze_type = MAP_RDMAZE_CATACOMBS "
  const int w = ( o->width >> 1 ) + rand () % 2;
  const int h = ( o->height >> 1 ) + rand () % 2;
  
  int places = 0b000; // TR, BL, BR
  const int offseti = rand () % 3;
  for ( int i = 0; i < 3 && places != 0b111; ++i ) {
    const int maze_type = rand () % 2; // MAP_RDMAZE_CATACOMBS or MAP_RDMAZE_ROOMS
    MAP_fill ( o, MAP_TYPE_VOID, 0, 0, w, h );
    MAP_rdmaze ( o, 0, 0, w, h, MAP_RDMAZE_DEPTH, maze_type );
    
    // this is dark magic ok?
    int currect_places = ( 1 << ( i + offseti ) % 3 );
    currect_places |= ( rand () % 2 << 2 ) | ( rand () % 2 << 1 ) | ( rand () % 2 << 0 );
    currect_places ^= places;
    places |= currect_places;
    
    if ( currect_places & 0b001 ) { // BR
      const int mx = rand () % 2 == 0 ? MAP_MIRROR_MIRROR : MAP_MIRROR_PASTE;
      const int my = rand () % 2 == 0 ? MAP_MIRROR_MIRROR : MAP_MIRROR_PASTE;
      MAP_mirror ( o, mx, my );
    }
    
    if ( currect_places & 0b010 ) { // BL
      const int mx = rand () % 2 == 0 ? MAP_MIRROR_MIRROR : MAP_MIRROR_PASTE;
      const int my = MAP_MIRROR_NO;
      MAP_mirror ( o, mx, my );
    }
    
    if ( currect_places & 0b100 ) { // TR
      const int mx = MAP_MIRROR_NO;
      const int my = rand () % 2 == 0 ? MAP_MIRROR_MIRROR : MAP_MIRROR_PASTE;
      MAP_mirror ( o, mx, my );
    }
  }
  
  if ( rand () % 2 == 0 ) { // TL
    const int maze_type = rand () % 2; // MAP_RDMAZE_CATACOMBS or MAP_RDMAZE_ROOMS
    MAP_fill ( o, MAP_TYPE_VOID, 0, 0, w, h );
    MAP_rdmaze ( o, 0, 0, w, h, MAP_RDMAZE_DEPTH, maze_type );
  }
  
  MAP_cross ( o, rand () % 2 == 0 ? MAP_CROSS_PASS : MAP_CROSS_VOID );
  MAP_borders ( o, MAP_TYPE_WALL );
}


int
MAP_gen ( MAP_map_t *o, const texture_t *textures, size_t tlength ) {
  const int width = 9 + ( rand () % 8 << 1 ) + 1;
  const int height = 9 + ( rand () % 8 << 1 ) + 1;
  MAP_init ( o, width, height );
  MAP_fill ( o, MAP_TYPE_VOID, 0, 0, o->width, o->height );
  
  // if ( rand () % 2 == 0 ) {
  if ( 0 ) { // TODO: no quadmap for now
    MAP_quadmap ( o );
  }
  else {
    const int maze_type = rand () % 2; // MAP_RDMAZE_CATACOMBS or MAP_RDMAZE_ROOMS
    MAP_fill ( o, MAP_TYPE_VOID, 0, 0, o->width, o->height );
    MAP_rdmaze ( o, 0, 0, o->width, o->height, MAP_RDMAZE_DEPTH, maze_type );
    MAP_borders ( o, MAP_TYPE_WALL );
  }
  
  // main textures
  int t1 = ( rand() % 16 ) <<  6;
  int t2 = ( rand() % 16 ) << 10;
  for ( int i = 0; i < o->length; ++i ) {
    if ( o->array[ i ] == MAP_TYPE_WALL ) {
      o->array[ i ] = 0; // flush
      o->array[ i ] |= 0x10; // make solid
      o->array[ i ] |= MAP_BTYPE_WALL; // make wall
    }
    else if ( o->array[ i ] == MAP_TYPE_PASS ) { // can be door
      o->array[ i ] = 0; // flush
      int x = i % width;
      int y = i / width;
      if ( o->array[ ( y - 1 ) * width + ( x     ) ] & 0xf == 1
      &&   o->array[ ( y + 1 ) * width + ( x     ) ] & 0xf == 1
      &&   o->array[ ( y     ) * width + ( x - 1 ) ] & 0xf != 1
      &&   o->array[ ( y     ) * width + ( x + 1 ) ] & 0xf != 1
      ||
           o->array[ ( y - 1 ) * width + ( x     ) ] & 0xf != 1
      &&   o->array[ ( y + 1 ) * width + ( x     ) ] & 0xf != 1
      &&   o->array[ ( y     ) * width + ( x - 1 ) ] & 0xf == 1
      &&   o->array[ ( y     ) * width + ( x + 1 ) ] & 0xf == 1 )
      {
        // really can be door
        o->array[ i ] |= 0x10; // make solid
        o->array[ i ] |= 0x20; // make transparent
        o->array[ i ] |= MAP_BTYPE_DOOR; // make door 
      }
    }
    o->array[ i ] |= t1;
    o->array[ i ] |= t2;
  }
  
  // random quads
  for ( int i = 0; i < rand() % 10; ++i ) {
    int x = rand() % width;
    int y = rand() % height;
    int w = rand() % width / 2;
    int h = rand() % height / 2;
    t1 = ( rand() % 16 ) <<  6;
    t2 = ( rand() % 16 ) << 10;
    for ( int yi = y; yi < h + y; ++yi ) {
      for ( int xi = x; xi < w + x; ++xi ) {
        o->array[ yi * o->width + xi ] |= t1;
        o->array[ yi * o->width + xi ] |= t2;
      }
    }
  }
  
  return 0;
}


// [ceiling texture:16(4)][floor/wall texture:16(4)][transparent:2(1)][solid:2(1)][type:16(4)]
#define MAP_BTYPE_VOID        0
#define MAP_BTYPE_WALL        1
#define MAP_BTYPE_DOOR        2
#define MAP_BTYPE_OPENDOOR    3
#define MAP_BTYPE_OPENINGDOOR 5
// #define MAP_BTYPE_REDDOOR     6
// #define MAP_BTYPE_BLUEDOOR    7
// #define MAP_BTYPE_GREENDOOR   8
// #define MAP_BTYPE_CYANDOOR    9


int
MAP_isempty ( const MAP_type_t v ) {
  const int type = ( v & 0xf );
  return type == MAP_BTYPE_VOID;
}









#endif // MAP_H

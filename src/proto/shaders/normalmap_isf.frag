/******************************************************************
    This file is part of DomeSimulator.

    DomeSimulator is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DomeSimulator is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DomeSimulator.  If not, see <http://www.gnu.org/licenses/>.

    DomeSimulator is free for non-commercial use. If you want to use it 
    commercially, you should contact the author 
    Michael Winkelmann aka Wilston Oreo by mail:
    me@wilstonoreo.net
**************************************************************************/

uniform sampler2DRect image; // Texture image
uniform sampler2DRect normal_map;

#define MAP_SPHERICAL 0
#define MAP_FISHEYE 1
#define MAP_CUBE 2

vec3 normal;

const float PI = 3.14159265358979323846264;

/// Convert degrees to radians
float deg2rad(in float deg)
{
  return deg * PI / 180.0;
}

uniform int map_mode;

uniform float map_yaw; // Any value allowed
uniform float map_pitch; // Any value allowed
uniform float map_roll; // Any value allowed

/// Calculates the rotation matrix of a rotation around X axis with an angle in radians
mat3 rotateAroundX( in float angle )
{
  float s = sin(angle);
  float c = cos(angle);
  return mat3(1.0,0.0,0.0,
              0.0,  c, -s,
              0.0,  s,  c);
}

/// Calculates the rotation matrix of a rotation around Y axis with an angle in radians
mat3 rotateAroundY( in float angle )
{
  float s = sin(angle);
  float c = cos(angle);
  return mat3(  c,0.0,  s,
              0.0,1.0,0.0,
               -s,0.0,  c);
}

/// Calculates the rotation matrix of a rotation around Z axis with an angle in radians
mat3 rotateAroundZ( in float angle )
{
  float s = sin(angle);
  float c = cos(angle);
  return mat3(  c, -s,0.0,
                s,  c,0.0,
              0.0,0.0,1.0);
}


/// Calculate rotation by given yaw and pitch angles (in degrees!)
mat3 rotationMatrix(in float yaw, in float pitch, in float roll)
{
  return rotateAroundZ(deg2rad(yaw)) * 
         rotateAroundY(deg2rad(-pitch)) * 
         rotateAroundX(deg2rad(roll));
}

vec3 map_normal()
{
  return rotationMatrix(map_yaw,map_pitch,map_roll) * normal;
}

float map_spherical(out vec2 texCoords) 
{
  vec3 normal = map_normal();
  texCoords.s = 1.0 - fract(atan(normal.x,normal.y) / (2.0*PI) - 0.5);
  texCoords.t = 1.0 - fract(acos(normal.z) / PI);

  texCoords.t = fract(texCoords.y);
  texCoords.s = fract(texCoords.x);
  return 1.0;
}
float map_fisheye(out vec2 texCoords)
{
  vec3 n = map_normal();
  float phi = atan(sqrt(n.x*n.x + n.y*n.y),-n.z);
  float r =  phi / PI * 2.0;
  if ((r > 1.0) || (r <= 0.0)) return -1.0;
  float theta = atan(n.x,n.y);
  texCoords.s = fract(0.5 * (1.0 + r* sin(theta)));
  texCoords.t = fract(0.5 * (1.0 + r * cos(theta)));
  return 1.0; 
}

float map_cube(out vec2 texCoords)
{
  vec3 n = map_normal();
  float sc, tc, ma;
  float eps =  -0.02;
  float _off = 0.0;
  //n = n.yzx;

  if ((abs(n.x) >= abs(n.y)) && (abs(n.x) >= abs(n.z)))
  {
    sc = (n.x > 0.0) ? -n.y : n.y;
    tc = n.z;
    ma = n.x;
    _off += (n.x < 0.0) ? 0.0/6.0 : 2.0/6.0; // EAST / WEST
  } else
  if ((abs(n.y) >= abs(n.z)))
  {
    sc = (n.y < 0.0) ? -n.x : n.x;
    tc = n.z;
    ma = n.y;
    _off += (n.y < 0.0) ? 1.0/6.0 : 3.0/6.0; // NORTH / SOUTH
  } else
  {
    sc = (n.z > 0.0) ? n.y : n.y;
    tc = (n.z > 0.0) ? n.x : -n.x;
//    tc = -n.x; 
    ma = n.z;
    _off = (n.z < 0.0) ? 4.0/6.0 : 5.0 / 6.0;  // TOP / BOTTOM
  }
  texCoords = vec2(sc/(12.0 - eps)/abs(ma) + 0.5/6.0 + _off,0.5+tc/(2.0 - eps)/abs(ma)) ;
  return 1.0;
}

float mapping(out vec2 texCoords)
{ 
  if (map_mode == MAP_SPHERICAL)
  {
    return map_spherical(texCoords);
  }
  if (map_mode == MAP_FISHEYE)
  {
    return map_fisheye(texCoords);
  }
  if (map_mode == MAP_CUBE)
  {
    return map_cube(texCoords);
  }
  return -1.0;
}

void main()
{
 // gl_FragColor = texture2DRect(image,gl_TexCoord[0].xy*vec2(3600.0,1800.0));

  gl_FragColor = vec4(0.0,0.0,0.0,1.0);
  vec2 uv = gl_TexCoord[0].st*vec2(1.0,1.0)*vec2(1920.0,1080.0);
  vec3 n = texture2DRect(normal_map,uv).xyz*2.0-1.0 + texture2DRect(normal_map,uv + vec2(0.0,1.0)*vec2(1920.0,1080.0)).xyz*2.0/255.0 - 1.0 / 255.0;
  if (length(n) <= 0.01) 
  {
    return;
  }
  normal = normalize(n);
  
  vec2 texCoords;
  if (mapping(texCoords) < 0.0)
  {
    return;
  }

  texCoords *= vec2(3600.0,1800.0);
  gl_FragColor = texture2DRect(image, texCoords); 
}


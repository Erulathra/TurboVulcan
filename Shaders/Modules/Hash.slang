module Hash;

import Math;

#include "MathConstants.slang"

public uint WangHash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

#define HashFunc WangHash

public float GetRandFloat(inout uint seed)
{
   const uint oldSeed = seed;
   seed = HashFunc(seed);

   return float(oldSeed) / float(UINT_MAX);
}

/* Stolen form https://gist.github.com/Piratkopia13/6db0896218e5d83479d38b0e43ab2b68 */
public float3 GetRandomDirectionInCone(inout uint seed, float3 dir, float angle)
{
   float cosAngle = cos(angle);

   // Generate points on the spherical cap around the north pole.
   // See https://math.stackexchange.com/a/205589/81266
   float z = GetRandFloat(seed) * (1.f - cosAngle) + cosAngle;
   float phi = GetRandFloat(seed) * M_2PI;

   float x = sqrt(1.f - z * z) * cos(phi);
   float y = sqrt(1.f - z * z) * sin(phi);
   float3 north = float3(0.f, 0.f, 1.f);

   // Find the rotation axis `u` and rotation angle `rot`
   float3 axis = normalize(cross(north, normalize(dir)));
   float angle = acos(dot(normalize(dir), north));

   // Convert rotation axis and angle to 3x3 rotation matrix
   float3x3 R = RotateAroundAxis(axis, angle);

   return mul(R, float3(x, y, z));
}

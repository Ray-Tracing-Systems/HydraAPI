#ifndef RANDOM_SIMPLE_RANDOM
#define RANDOM_SIMPLE_RANDOM

namespace simplerandom
{
  
  struct uint2
  {
    unsigned int x;
    unsigned int y;
  };
  
  typedef struct RandomGenT
  {
    uint2 state;
    unsigned int maxNumbers;
    unsigned int lazy; // or dummy to have uint4 generator data.
    
  } RandomGen;
  
  
  static inline unsigned int NextState(RandomGen *gen)
  {
    const unsigned int x = (gen->state).x * 17 + (gen->state).y * 13123;
    (gen->state).x = (x << 13) ^ x;
    (gen->state).y ^= (x << 7);
    return x;
  }
  
  
  static inline RandomGen RandomGenInit(const int a_seed)
  {
    RandomGen gen;
    
    gen.state.x = (a_seed * (a_seed * a_seed * 15731 + 74323) + 871483);
    gen.state.y = (a_seed * (a_seed * a_seed * 13734 + 37828) + 234234);
    gen.lazy    = 0;
    
    for (int i = 0; i < (a_seed % 7); i++)
      NextState(&gen);
    
    return gen;
  }
  
  static inline float rndFloat(RandomGen *gen)
  {
    const unsigned int x   = NextState(gen);
    const unsigned int tmp = (x * (x * x * 15731 + 74323) + 871483);
    const float scale      = (1.0f / 4294967296.0f);
    return ((float) (tmp)) * scale;
  }
  
  static inline float rnd(RandomGen& gen, float s, float e)
  {
    const float t = rndFloat(&gen);
    return s + t*(e - s);
  }
  
  static inline unsigned int rand(RandomGen& gen)
  {
    return NextState(&gen);
  }
  
  // static inline float4 rndFloat4_Pseudo(RandomGen *gen)
  // {
  //   unsigned int x = NextState(gen);
  //
  //   const unsigned int x1 = (x * (x * x * 15731 + 74323) + 871483);
  //   const unsigned int y1 = (x * (x * x * 13734 + 37828) + 234234);
  //   const unsigned int z1 = (x * (x * x * 11687 + 26461) + 137589);
  //   const unsigned int w1 = (x * (x * x * 15707 + 789221) + 1376312589);
  //
  //   const float scale = (1.0f / 4294967296.0f);
  //
  //   return make_float4((float) (x1), (float) (y1), (float) (z1), (float) (w1)) * scale;
  // }
  //
  // static inline float2 rndFloat2_Pseudo(RandomGen *gen)
  // {
  //   unsigned int x = NextState(gen);
  //
  //   const unsigned int x1 = (x * (x * x * 15731 + 74323) + 871483);
  //   const unsigned int y1 = (x * (x * x * 13734 + 37828) + 234234);
  //
  //   const float scale = (1.0f / 4294967296.0f);
  //
  //   return make_float2((float) (x1), (float) (y1)) * scale;
  // }
  
};

#endif


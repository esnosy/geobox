#pragma once

#include <random>

template <typename Random_Device_Type, typename Random_Engine_Type, typename Distribution_Type> class Random_Generator {
private:
  Random_Engine_Type m_random_engine;
  Distribution_Type m_distribution;

public:
  Random_Generator(Random_Device_Type &random_device, const Distribution_Type &distribution)
      : m_random_engine(random_device()), m_distribution(distribution) {}

  auto generate() { return m_distribution(m_random_engine); }
};


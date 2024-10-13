# Settings

[TOC]

## TL;DR
- Parameters in simulations are specified in a YAML file.
- The location of the YAML file should be given to the program when executing, using `---setting` (or `-s`) option.


## Specification
### Computational Conditions
These parameters are used to specify the computational conditions.
All the parameters are required, and must be written in the top level of the YAML file.
- `dim`: Dimension of the simulation. It must be 2 or 3.
- `particleDistance`: Distance between particles at inital particle arrangement. It is used to calculate the number of particles.
- `dt`: Time step of the simulation.
- `endTime`: End time of the simulation.
- `outputPeriod`: Interval to output the results.
- `cflCondition`: Maximum CFL number. If the CFL number exceeds this value, It emits a warning message.
- `numPhysicalCores`: Number of physical cores to use. It is a positive integer. If OpenMP is not available, it is ignored.
- `radiusRatio`: Ratio of the effective radius in the Weight function to be used in the simulation. It is a positive real number.
    - `forNumberDensity`: Ratio of the effective radius in the Weight function to be used in the number density calculation.
    - `forGradient`: Ratio of the effective radius in the Weight function to be used in the gradient operator.
    - `forLaplacian`: Ratio of the effective radius in the Weight function to be used in the Laplacian operator.

### Domain
It specifies the domain of the simulation. It is required, and must be written in the `domain` section.
- `min`: Minimum coordinates of the domain. It is a three-dimensional vector.
- `max`: Maximum coordinates of the domain. It is a three-dimensional vector.

If the domain is two-dimensional, the third component of the vectors must be zero.

### Physical Properties
These properties are used to calculate the pressure and the density of the fluid.
All the properties are required, and must be written in the `physicalProperties` section.
- `defaultDensity`: Default density of fluid.
- `kinematicViscosity`: Kinematic viscosity of fluid. Kinematic viscosity is the ratio of dynamic viscosity to density.
- `gravity`: Gravitational acceleration. It is a three-dimensional vector.

### Surface Detection Method
In this code, several surface detection methods are implemented.
The user must choose *one* of them in the `surfaceDetection` section.
Folliwing methods are available:
- `NumberDensity`: It detects the surface particles based on the number density of the each particle.
- `particleDistribution`: It detects the surface particles based on the distribution of the particles.

For details, see [Surface Detection Methods](../improved_algorithms/surface_detection.md).

### Pressure Calculation Method
In this code, two types of pressure calculation methods are implemented.
The user must choose *one* of them in the `pressureCalculation` section.
- `Implicit`: It calculates the pressure implicitly. It calculates the pressure by solving the Poisson equation.
- `Explicit`: It calculates the pressure explicitly. It calculates the pressure by using the equation of state.

### Stabilization Method
In this code, collison based stabilization method is implemented.
Other stabilization methods are not implemented yet.
User must set parameters for the collision based stabilization method in the `stabilization` section.
- `collision`: It is a collision based stabilization method.
  - `collisionDistance`: The distance to detect the collision.
  - `collisionCoefficient`: The coefficient to stabilize the collision.

### Inital Particle Arrangement
The initial particles arrangement is specified by the `.prof` file.
The location of the `.prof` file must be specified in the 
`particlesPath` at the top level of the YAML file.
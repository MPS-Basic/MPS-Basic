# Input/Output

[TOC]

## Input
### Input Files
Two input files are required.

- Particles Data: [Profile data](#profile) or [CSV data](#csv) to specify particles positions at the beginning of the simulation. Only CSV data is supported for simulation of multiple fluid types.
- Settings: YAML data `***.yml` to specify simulation settings

@note
You can see `input/***/***.vtu`, but this is not used for simulation.
This is a ParaView file just for checking if the input file is correctly generated.

### Specifying Locations
- The location of particles data should be written in `***.yml` file.
- The location of `***.yml` should be given to the program when executing, using `---setting` (or `-s`) option.
  ```powershell
  ./build/mps.exe --setting input/dambreak/settings.yml --output result/dambreak
  ```

### Generating Input Files
#### YAML file
`***.yml` should be modified by the user based on ther desired simulation settings.

#### Prof data
`***.prof` can be made by cpp programs provided in `generator` folder.
Currently `generate_dambreak.cpp` and `generate_hydrostatic.cpp` are provided,
but users can add their own generators.
`generate_***.cpp` is built at the same time as the main program is built,
so build the main program first following [Execution page](execution.md).
Then you will see `build/generator/generate_***.exe`, so execute this.

Example of making `input/dambreak/input.prof` for dambreak in Windows:
```powershell
./build/generator/generate_dambreak.exe
```

Now you will see `input/dambreak/input.vtu`.
Open this in ParaView to check if the input file is generated correctly.

## Output
- The results are written in the following formats:
	- `result/prof`: [Profile data](#profile)
	- `result/vtu`: VTK data

## Data Syntax
### Profile {#profile}
- The profile data is in the following format:

```prof
0  // time
627  // number of particles
3 -0.1 -0.1 0 0 0 0  // type x y z u v w
3 -0.1 -0.075 0 0 0 0
3 -0.1 -0.05 0 0 0 0
...
```

### CSV {#csv}
- The CSV data is in the following format:

```csv
0  // time
627  // number of particles
type,x,y,z,vx,vy,vz,fluidType
3,-0.1,-0.1,0,0,0,0,0
3,-0.1,-0.075,0,0,0,0,0
3,-0.1,-0.05,0,0,0,0,0
...
```

- Header is required for CSV data.
- The order of columns can be changed.
- fluidType is optional. In single phase simulation, it is not used.

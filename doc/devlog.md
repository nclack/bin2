# 2024-08-10

Having lots of trouble with tests :)

- for avx and neon code 
  - input/output pointer needs to be aligned to lane count
  - width needs to be divisible by 4x lane count with a 2x unrolling

That width restriction is a lot so probably want to do some smart dispatch.
It might help to do the refactor for the n-dimensional part first.
 

## Questions

- for memory bandwidth, is it better to use in-place or out-of-place? How much
  of a difference does it make?

## Todo ideas

- server that helps w debugging the images. separate repo, compose with flakes,
  spin it up and enable in the dev env.
- make bin2x2 n-dimensional

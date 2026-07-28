# Mission: Translate EzDesign topology conventions into OCCT

## Why
Make reliable converter decisions when EzDesign and OCCT encode the same
topological intent in different places, especially across STEP round trips.

## Success looks like
- Identify which EzDesign field maps to OCCT face, wire, and edge orientation.
- Predict the oriented face normal after `MakeFace` and STEP round-trip.
- Place orientation corrections at the narrowest correct topological level.

## Constraints
- Assume expert knowledge of topology; teach convention differences, not basics.
- Ground claims in official OCCT documentation, source, and focused probes.
- Preserve EzDesign's stated outward-normal and loop-winding convention.

## Out of scope
- General B-rep or differential-geometry instruction.
- Periodic surfaces, seams, and inner-loop policy until a fixture requires them.

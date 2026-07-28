# EzDesign–OCCT Orientation Resources

## Knowledge

- [Local research: OCCT face orientation and `is_surface_normal_same`](docs/research/occt-face-orientation.md)
  Primary-source synthesis and OCCT 7.9.3 STEP round-trip probe. Use for the
  converter conclusion and regression design.
- [OCCT 7.9.3 Modeling Data: Orientation](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/dox/user_guides/modeling_data/modeling_data.md#L650-L678)
  Defines default regions and `FORWARD`/`REVERSED`. Use for the semantic model.
- [OCCT 7.9.3 `BRepBuilderAPI_MakeFace`](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepBuilderAPI/BRepBuilderAPI_MakeFace.hxx#L37-L58)
  Defines `Inside=true` as orienting a wire to bound a finite surface area.
- [OCCT 7.9.3 `BRepLib_MakeFace::CheckInside`](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepLib/BRepLib_MakeFace.cxx#L820-L841)
  Shows that finite-area normalization reverses child wires, not the face.
- [OCCT 7.9.3 STEP face writer](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/TopoDSToStep/TopoDSToStep_MakeStepFace.cxx#L438-L454)
  Shows how face orientation becomes `ADVANCED_FACE.same_sense`.

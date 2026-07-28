# OCCT face orientation and `is_surface_normal_same`

Date: 2026-07-28

Repository revision examined: `bf72ff31fabef648df495fa8161ba4c2ddf2c295`

OCCT version examined and probed: 7.9.3

## Conclusion

The current `is_surface_normal_same == false` conversion is **not correct**.
`BRepBuilderAPI_MakeFace(surface, wire, Standard_True)` makes the wire bound a
finite UV area, but it does not make the face normal agree with EzDesign's face
normal. For a closed clockwise outer wire it reverses the boundary and leaves
the face `FORWARD`, so the resulting OCCT face normal still follows the surface
normal—the opposite of the EzDesign face normal.

The required correction is to reverse the completed face when
`is_surface_normal_same` is false:

```cpp
TopoDS_Face resultFace = faceMaker.Face();
if (!theFace.is_surface_normal_same) {
  resultFace.Reverse();
}
```

This is a conclusion, not an implementation in this research change. Do **not**
reverse the input wire instead: `Inside=true` normalizes a closed outer wire
back to the finite-area orientation, while the face remains `FORWARD`. Do not
remove the flag; it carries the information needed to choose the face
orientation.

The true case is already correct, assuming the stated EzDesign convention and
valid pcurves: a UV-counter-clockwise outer loop bounds the finite region and a
`FORWARD` OCCT face follows the surface normal.

## Evidence

### Documented facts

1. OCCT defines the default material region of a surface boundary as the region
   to the left of the naturally directed edge, specifically the direction
   `surface normal × curve tangent`. For a face bounding space, the default
   region is on the negative side of the surface normal. `FORWARD` keeps the
   default region and `REVERSED` keeps its complement. This makes face
   orientation—not wire winding—the topological carrier of surface-normal
   versus face-normal sense. [OCCT 7.9.3 Modeling Data, “Orientation”,
   lines 650–678](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/dox/user_guides/modeling_data/modeling_data.md#L650-L678)

2. The documented meaning of `Inside=true` is only that “the wire is oriented
   to bound a finite area on the surface.” It does not say that the face is
   reversed to match an application-supplied outward normal.
   [OCCT 7.9.3 `BRepBuilderAPI_MakeFace.hxx`, lines
   37–58](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepBuilderAPI/BRepBuilderAPI_MakeFace.hxx#L37-L58)
   The surface-and-wire overload also requires every edge to carry a pcurve on
   a non-planar surface. [OCCT 7.9.3 `BRepBuilderAPI_MakeFace.hxx`, lines
   169–176](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepBuilderAPI/BRepBuilderAPI_MakeFace.hxx#L169-L176)

3. `BRepBuilderAPI_MakeEdge(pcurve, surface, V1, V2, p1, p2)` stores the 2D
   curve as the edge's parametric curve on the surface. Parameters are expected
   in increasing order; decreasing parameters cause OCCT to swap the vertices
   and mark the edge `REVERSED`. There is no documented face-normal or
   outer-wire inference in this operation. [OCCT 7.9.3
   `BRepBuilderAPI_MakeEdge.hxx`, lines
   193–232](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepBuilderAPI/BRepBuilderAPI_MakeEdge.hxx#L193-L232)

4. `TopoDS_Shape::Reverse()` exchanges `FORWARD` and `REVERSED` orientation,
   and OCCT defines that exchange as swapping material sides.
   [OCCT 7.9.3 `TopoDS_Shape.hxx`, lines
   216–227](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/TopoDS/TopoDS_Shape.hxx#L216-L227)
   [OCCT 7.9.3 `TopAbs.hxx`, lines
   72–82](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/TopAbs/TopAbs.hxx#L72-L82)

### Source-code facts

1. The `Geom_Surface + TopoDS_Wire + Inside` builder creates a face, adds the
   wire, and calls `CheckInside()` only when `Inside` is true and the wire is
   closed. [OCCT 7.9.3 `BRepLib_MakeFace.cxx`, lines
   327–333](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepLib/BRepLib_MakeFace.cxx#L327-L333)

2. `CheckInside()` classifies the infinite UV point. If infinity is inside, it
   makes an empty copy of the face and adds every child with reversed
   orientation. It does not call `Reverse()` on the face. `EmptyCopied()`
   explicitly preserves the original orientation, so this changes the wire
   orientation while leaving the face orientation unchanged.
   [OCCT 7.9.3 `BRepLib_MakeFace.cxx`, lines
   820–841](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepLib/BRepLib_MakeFace.cxx#L820-L841)
   [OCCT 7.9.3 `TopoDS_Shape.hxx`, lines
   294–306](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/TopoDS/TopoDS_Shape.hxx#L294-L306)

3. Adding later wires (holes) is a plain `BRep_Builder::Add`; it does not repeat
   `CheckInside()`. The first closed wire passed to the constructor is therefore
   the wire whose finite-area sense `Inside=true` normalizes.
   [OCCT 7.9.3 `BRepLib_MakeFace.cxx`, lines
   790–795](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepLib/BRepLib_MakeFace.cxx#L790-L795)

4. The pcurve edge implementation preserves the supplied curve direction for
   `p1 < p2`: it assigns `VV1`/`VV2` in that order, evaluates the surface at the
   pcurve endpoints, stores the pcurve and range, and reverses the edge only
   when it had to reorder decreasing parameters.
   [OCCT 7.9.3 `BRepLib_MakeEdge.cxx`, lines
   878–1058](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepLib/BRepLib_MakeEdge.cxx#L878-L1058)

5. OCCT's own face-normal calculation uses `D1U × D1V` for a `FORWARD` face
   and negates it for a `REVERSED` face.
   [OCCT 7.9.3 `BRepGProp_Face.cxx`, lines
   184–203](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepGProp/BRepGProp_Face.cxx#L184-L203)

6. The STEP writer writes `ADVANCED_FACE.same_sense` from the OCCT face
   orientation (`FORWARD` means true), and the reader restores the OCCT face as
   `FORWARD` or `REVERSED` from `same_sense`. It also adjusts wire orientation
   separately, so face sense and bound sense are intentionally distinct.
   [OCCT 7.9.3 `TopoDSToStep_MakeStepFace.cxx`, lines
   438–454](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/TopoDSToStep/TopoDSToStep_MakeStepFace.cxx#L438-L454)
   [OCCT 7.9.3 `StepToTopoDS_TranslateFace.cxx`, lines
   713–755](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/StepToTopoDS/StepToTopoDS_TranslateFace.cxx#L713-L755)

7. In this repository the flag is parsed and documented as the relation between
   surface normal and face outward normal, but is ignored by
   `convertHalfEdge()`. `convertFace()` always returns the face produced by
   `MakeFace` unchanged.
   [`EzDesignTypes.hxx`, lines 90–100](../../src/EzDesignTypes.hxx#L90-L100)
   [`EzDesignJsonReader.cxx`, lines
   419–425](../../src/EzDesignJsonReader.cxx#L419-L425)
   [`EzDesignToOCCTConverter.cxx`, lines
   136–196](../../src/EzDesignToOCCTConverter.cxx#L136-L196)
   [`EzDesignToOCCTConverter.cxx`, lines
   251–278](../../src/EzDesignToOCCTConverter.cxx#L251-L278)

### Experimental inference

A disposable OCCT 7.9.3 C++ probe used a `Geom_Plane` whose geometric normal
was `+Z`. Four edges were created from 2D lines with
`BRepBuilderAPI_MakeEdge(pcurve, surface, start, end, 0, length)` and assembled
as the clockwise UV square
`(0,0) → (0,1) → (1,1) → (1,0) → (0,0)`. This models the false case: the
intended face normal is `-Z`, and the same physical outer loop is clockwise
when viewed along the surface normal.

Each case was checked with `BRepCheck_Analyzer`, one-wire/four-edge counts,
`BRepClass_FaceClassifier` at `(0.5,0.5)` and `(2,2)`, signed UV area of the
outer wire in face context, and the oriented normal
`D1U × D1V` (negated for `REVERSED`). Pcurve-only edges were given 3D curves
with `BRepLib::BuildCurves3d` before validity checking and STEP transfer.

| Case | Face orientation | Contextual outer UV area | Oriented normal Z |
| --- | --- | ---: | ---: |
| CW wire, `Inside=true` | `FORWARD` | `+1` | `+1` |
| Reversed CW wire, `Inside=true` | `FORWARD` | `+1` | `+1` |
| First result, then face reversed | `REVERSED` | `-1` | `-1` |
| Raw face after STEP round trip | `FORWARD` | `+1` | `+1` |
| Reversed face after STEP round trip | `REVERSED` | `-1` | `-1` |

All five topology/inside checks passed. The raw and corrected STEP files
contained `ADVANCED_FACE(...,.T.)` and `ADVANCED_FACE(...,.F.)`, respectively.
After reimport OCCT restored the same face orientation, contextual boundary
winding, and geometric normal. These results confirm the source reading:
`Inside=true` fixes bounded-area winding, while face reversal carries the
surface-normal/face-normal opposition through STEP.

The probe was run outside the repository and is not retained.

## Why the alternatives are wrong

- **Leave the false case unchanged:** topology is bounded, but the face normal
  remains the surface normal, opposite the required EzDesign normal.
- **Reverse the wire before `MakeFace(..., Inside=true)`:** `CheckInside()`
  normalizes the closed wire back to finite-area orientation; the face remains
  `FORWARD`. The probe reproduced this.
- **Use `Inside=false`:** this can preserve the supplied winding, but it still
  leaves the face `FORWARD`; it also gives up the constructor's finite-area
  normalization. It does not encode the normal relationship.
- **Reverse every pcurve/edge:** pcurve direction already represents the
  EzDesign half-edge direction, and individual reversal would interfere with
  shared-edge orientation. The normal relationship belongs one level higher,
  on the face.

## Minimal regression fixture

Use two variants of one one-face, one-loop planar B-spline fixture:

1. **Same case:** surface derivatives give `+Z`,
   `is_surface_normal_same: 1`, outer pcurves traverse a unit square
   counter-clockwise in UV, expected face normal `+Z`.
2. **Opposite case:** use the same surface,
   `is_surface_normal_same: 0`, outer pcurves traverse the square clockwise in
   UV, expected face normal `-Z`.

The opposite variant is the required new coverage; the same variant prevents a
fix from reversing every face. Use distinct edge IDs in each fixture so the
test isolates face sense rather than shared-edge behavior.

For each variant:

1. Convert with the installed CLI and export STEP.
2. Reimport with `STEPControl_Reader`; require successful root transfer and
   exactly one face.
3. Require one outer wire, four edges, `BRepCheck_Analyzer(face).IsValid()`,
   center UV state `IN`, and an exterior UV point state `OUT`. Counts alone are
   insufficient.
4. Evaluate the surface at the center. Compute `D1U × D1V`, apply the surface
   location, and reverse the vector when `face.Orientation()` is
   `TopAbs_REVERSED`. Require its dot product with the expected EzDesign face
   normal to exceed `0.999`.
5. Walk `BRepTools::OuterWire(face)` with `BRepTools_WireExplorer`, evaluate
   each pcurve in its contextual edge orientation, and check signed UV area:
   positive for the same case and negative for the opposite case after STEP
   reimport.

The disposable probe already demonstrated these assertions on equivalent OCCT
topology and through STEP. The remaining implementation test should exercise
the JSON reader and converter with the actual minimal `.ezd` fixture before
changing production code.

## Scope note

This conclusion establishes the outer boundary and face-normal behavior asked
about. If EzDesign files with holes are supported, add one later fixture with an
inner loop: `BRepLib_MakeFace::Add()` does not normalize added wires, so their
documented EzDesign winding must be checked independently. That does not change
the false-case face reversal conclusion.

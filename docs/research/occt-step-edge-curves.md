# OCCT pcurve-only edges and STEP export

Date: 2026-09-02

Repository revision examined: `3b13d4144f8f4feefde0528d98c59dc54ed53220`

OCCT version examined: 7.9.3

## Conclusion

Yes: the missing OCCT 3D edge curve is a strong, concrete explanation for the
reported wrong-looking boundaries. A repository-specific probe also found a
second definite bug: the second face's pcurve is stored with the opposite
parameter sense from the shared base edge.

The current converter creates each non-fallback edge from one 2D pcurve and its
surface, and deliberately leaves out the 3D curve on the assumption that the
STEP writer will compute it. The writer does produce something, but its
missing-3D-curve fallback is a fixed 21-point, degree-1 B-spline: a polyline
through 21 samples of the pcurve lifted onto the current face. It is not
`BRepLib::BuildCurves3d`, is not driven by a geometric tolerance, and does not
update or validate the OCCT edge. On a curved surface this can visibly change
the boundary shape. [Current converter, lines
332–374](../../src/EzDesignToOCCTConverter.cxx#L332-L374)
[OCCT 7.9.3 STEP edge translation, lines
183–338](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/TopoDSToStep/TopoDSToStep_MakeStepEdge.cxx#L183-L338)

On `tests/data/20260427.ezd`, both shared edges' two lifted pcurves trace the
same spatial locus, but they agree only when the second parameter is reversed.
The current code adds that face-oriented second pcurve directly to the natural
base edge and then returns a reversed edge occurrence. The occurrence reversal
is topologically right, but the stored pcurve must first be reversed into the
base edge's natural parameter direction. [Current converter, lines
263–279](../../src/EzDesignToOCCTConverter.cxx#L263-L279)
[Current `addPCurveToEdge`, lines
378–446](../../src/EzDesignToOCCTConverter.cxx#L378-L446)

Missing 3D geometry is therefore real, but it is not the sole issue. The two
face pcurves must describe the same spatial locus, have compatible
orientation/ranges, and be stored correctly for seam edges. Building a 3D
curve from a wrongly oriented first/second representation merely makes an
inconsistent edge explicit.

The smallest sound export pipeline is therefore:

1. finish all edge/pcurve associations in the underlying edge's natural
   orientation, including both seam pcurves;
2. call `BRepLib::BuildCurves3d(shape, exportTolerance)` and require `true`;
3. force `SameParameter` reconstruction with
   `BRepLib::SameParameter(shape, exportTolerance, Standard_True)`;
4. reject the pre-export shape unless exact `BRepCheck_Analyzer` and an
   application-tolerance deviation check pass;
5. export, transfer the STEP roots back to OCCT, and repeat the same checks on
   the imported shape.

Do not treat `STEPControl_Writer::Transfer() == IFSelect_RetDone` or
`STEPControl_Reader::ReadFile() == IFSelect_RetDone` as geometric validation.

## What an edge representation actually contains

The “three representations” mental model is broadly correct for a shared edge
between two parametric faces:

- a 3D curve carried by the topological edge;
- one pcurve in face A's UV domain;
- one pcurve in face B's UV domain.

The 3D curve is not inherently polygonal or inherently approximate. For a
planar support OCCT converts the pcurve to 3D directly. For a general surface,
the composed curve `S(P(t))` often has no convenient exact representation in
the target curve class, so `BRepLib::BuildCurve3d` builds a tolerance-controlled
3D approximation, by default requesting C1 continuity and permitting degree up
to 14. [OCCT 7.9.3 `BRepLib.hxx`, lines
68–108](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepLib/BRepLib.hxx#L68-L108)
[OCCT 7.9.3 `BRepLib.cxx`, lines
295–474](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepLib/BRepLib.cxx#L295-L474)

STEP also distinguishes these roles. `EDGE_CURVE.edge_geometry` is a required
`curve`. With surface-curve output, that geometry is a `SURFACE_CURVE`, whose
required `curve_3d` must be three-dimensional and whose
`associated_geometry` contains one or two pcurves/surfaces. A STEP `PCURVE`
refers to exactly one 2D curve on a basis surface. [STEP merged EXPRESS schema,
`edge_curve`](https://www.steptools.com/stds/stp_aim/html/t_edge_curve.html)
[STEP merged EXPRESS schema,
`surface_curve`](https://www.steptools.com/stds/stp_aim/html/t_surface_curve.html)
[STEP merged EXPRESS schema,
`pcurve`](https://www.steptools.com/stds/stp_aim/html/t_pcurve.html)
The governing ISO resource is ISO 10303-42, “Geometric and topological
representation.” [ISO 10303-42:2025 catalogue
entry](https://www.iso.org/standard/91386.html)

## What OCCT does with pcurve-only edges

### `BRepLib::BuildCurve3d` / `BuildCurves3d`

`BRepLib::BuildCurve3d(edge, tol, ...)`:

- returns immediately if a 3D curve already exists;
- normalizes representation ranges first if `CheckSameRange` fails;
- maps a pcurve on a plane directly to 3D;
- otherwise uses the first available pcurve/surface pair to run
  `GeomLib::BuildCurve3d` at the requested tolerance;
- does **not** establish that a second face's pcurve is spatially consistent
  with the first one.

`BuildCurves3d(shape, ...)` applies that operation once to each unique edge and
returns false if any edge fails. [OCCT 7.9.3 `BRepLib.cxx`, lines
295–474](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepLib/BRepLib.cxx#L295-L474)

That last point matters: the generated curve is based on whichever pcurve is
encountered first. The second pcurve is a constraint to verify afterward, not
an additional input blended into the approximation.

### `SameRange` and `SameParameter`

These names describe two different invariants:

- `SameRange`: all curve representations use the same parameter interval. If
  a 3D curve exists, its interval is the reference; otherwise the first curve
  representation supplies it.
- `SameParameter`: for every pcurve `P` on surface `S`, the same parameter `t`
  denotes the same spatial point as the 3D curve `C`, within edge tolerance:
  `|C(t) - S(P(t))| <= edgeTolerance`.

`SameParameter` is stronger than “the two curves trace the same shape”; it
also requires synchronized parameterization. [OCCT 7.9.3 `BRepLib.hxx`, lines
68–108 and 159–199](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepLib/BRepLib.hxx#L68-L199)
[OCCT 7.9.3 `BRepCheck_Analyzer.hxx`, lines
78–137](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepCheck/BRepCheck_Analyzer.hxx#L78-L137)

New OCCT `BRep_TEdge` objects initialize both flags to `true`; the flags are not
proof that a later-added pcurve was actually compared with a 3D curve. The
non-forced `BRepLib::SameParameter` skips an edge whose flag is already true.
The shape overload with `forced = Standard_True` first clears both flags and
recomputes them, which is why forcing is appropriate at this import boundary.
[OCCT 7.9.3 `BRep_TEdge.cxx`, lines
22–61](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRep/BRep_TEdge.cxx#L22-L61)
[OCCT 7.9.3 `BRepLib.cxx`, lines
864–968](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepLib/BRepLib.cxx#L864-L968)

Do not use a large stored edge/vertex tolerance as a substitute for correct
geometry. `BRepCheck` judges curve disagreement against the tolerance stored on
the edge, so inflating tolerance can make a wrong edge formally valid. The
converter currently expands vertex tolerance to 1.1 times each endpoint
discrepancy; a separate fixed application/export tolerance must therefore be
used as the acceptance threshold. [Current converter, lines
335–366](../../src/EzDesignToOCCTConverter.cxx#L335-L366)
[OCCT 7.9.3 `BRepCheck_Edge.cxx`, lines
300–438](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepCheck/BRepCheck_Edge.cxx#L300-L438)

### What `BRepCheck_Analyzer` catches

For a non-degenerate edge, absence of a 3D curve produces
`BRepCheck_No3DCurve`. In a face context it also checks the presence and truth
of `SameRange`/`SameParameter`, representation ranges, a pcurve on the face,
and the deviation between the 3D curve and each face pcurve. Exact mode uses
`BRepLib_CheckCurveOnSurface` when the edge is `SameParameter`; the default
mode samples a finite number of points. [OCCT 7.9.3 `BRepCheck_Edge.cxx`, lines
72–158 and 259–503](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepCheck/BRepCheck_Edge.cxx#L72-L503)
[OCCT 7.9.3 `BRepCheck_Analyzer.hxx`, lines
28–137](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepCheck/BRepCheck_Analyzer.hxx#L28-L137)

Useful focused checks are:

- `ShapeAnalysis_Edge::Curve3d` / `HasCurve3d`;
- `CheckVerticesWithCurve3d` and `CheckVerticesWithPCurve`;
- `CheckCurve3dWithPCurve` for endpoint sense;
- `CheckSameParameter(edge, maxDeviation, sampleCount)` for a numeric maximum
  over all pcurves;
- `ShapeAnalysis_Wire::CheckEdgeCurves`, `CheckClosed`, `CheckConnected`, and
  `CheckSelfIntersection` for each face wire.

The wire checker documents that `CheckEdgeCurves` includes 3D/pcurve sense,
vertices against both representations, seam checking, 2D/3D gaps, and
SameParameter. [OCCT 7.9.3 `ShapeAnalysis_Edge.hxx`, lines
151–277](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/ShapeAnalysis/ShapeAnalysis_Edge.hxx#L151-L277)
[OCCT 7.9.3 `ShapeAnalysis_Wire.hxx`, lines
146–317](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/ShapeAnalysis/ShapeAnalysis_Wire.hxx#L146-L317)

### Seam edges are a separate case

A seam is not the ordinary “one pcurve per adjacent face” case. The same
topological edge occurs twice on one closed face and needs two pcurves on that
same surface. OCCT represents those together with
`BRep_Builder::UpdateEdge(edge, pcurve1, pcurve2, face, tolerance)`;
`BRep_Tool::IsClosed(edge, face)` reports whether that dual representation
exists, and `CurveOnSurface` selects pcurve 1 or 2 from edge orientation.
[OCCT 7.9.3 `BRep_Builder.hxx`, lines
149–205](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRep/BRep_Builder.hxx#L149-L205)
[OCCT 7.9.3 `BRep_Tool.hxx`, lines
119–177 and 236–246](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRep/BRep_Tool.hxx#L119-L246)
[OCCT 7.9.3 `BRep_Tool.cxx`, lines
289–349 and 744–792](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRep/BRep_Tool.cxx#L289-L349)

The current converter cannot create that representation: when an edge ID is
seen again it asks only whether a pcurve already exists on the surface; if one
does, it returns the reversed edge without storing the second pcurve. This is a
second likely defect for periodic surfaces, independent of 3D-curve quality.
[Current converter, lines
263–280](../../src/EzDesignToOCCTConverter.cxx#L263-L280)

## What `STEPControl_Writer` checks and repairs

`STEPControl_Writer::Transfer` delegates to the STEP actor. In OCCT 7.9.3 the
writer's default processing flags are only `SplitCommonVertex` and
`DirectFaces`; it does not run `BRepLib::BuildCurves3d` as a precondition.
[OCCT 7.9.3 `STEPControl_Writer.cxx`, lines
70–128 and 185–211](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/STEPControl/STEPControl_Writer.cxx#L70-L211)

During individual edge translation, if a 3D curve is absent the STEP mapper:

- creates an exact 3D line only for a line on a plane;
- otherwise evaluates 21 equally spaced parameters and creates a degree-1
  B-spline through those points;
- wraps it as `SURFACE_CURVE` or `SEAM_CURVE` when pcurve output is enabled;
- always initializes `EDGE_CURVE.same_sense` to true in this mapper.

[OCCT 7.9.3 `TopoDSToStep_MakeStepEdge.cxx`, lines
183–338](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/TopoDSToStep/TopoDSToStep_MakeStepEdge.cxx#L183-L338)

Therefore `Transfer() == IFSelect_RetDone` means translation produced STEP
entities; it does not mean that the input B-rep satisfied OCCT geometry
invariants or that the fallback polyline met an application tolerance.

The executable's current “verification” calls only `ReadFile` and examines the
STEP model's load-time checks. It never calls `TransferRoots`, obtains an
imported `TopoDS_Shape`, or runs `BRepCheck_Analyzer`.
[Current CLI, lines 136–160](../../src/ezd2step.cxx#L136-L160)
The general conversion regression test does transfer roots, but then checks
only nonzero face/edge/vertex counts; it does not check B-rep validity or
geometric deviation. [Current conversion test, lines
48–85](../../tests/test_20260427_conversion.cxx#L48-L85)

## Reproduction on the repository fixture

### Supplied `20260902.ezd`

The supplied normal-save file contains a base64-encoded compact document, so
the payload was decoded in-memory before passing it to the converter (the STEP
temporary-file contract remains plain JSON). Before the fix, its 68 unique
edges all lacked a 3D curve. All 60 shared edges had their second pcurve in the
opposite sense; after accounting for that reversal, the two face curves agreed
spatially to `5.36e-9`.

Sixteen of those shared representations use trimmed bounds (`[0, 0.5]` or
`[0.5, 1]`) while the opposite representation uses `[0, 1]`. Calling
`Geom2d_BSplineCurve::Reverse()` but retaining the original trimmed bounds
selects the other half of the basis curve. The corrected conversion maps both
bounds through `ReversedParameter()` before reversing the curve.

After correcting sense and bounds, then running `BuildCurves3d`, forced
`SameParameter`, and exact `BRepCheck_Analyzer`, all 68 unique edge
representations validate. The resulting STEP contains 68 `SURFACE_CURVE`
entities backed by 24 degree-3 and 44 degree-6 3D B-splines, with no degree-1
3D fallback curves. OCCT reads and transfers it as 30 faces, 128 edge
occurrences, and 256 vertex occurrences.

### Checked-in `20260427.ezd` fixture

Two disposable probes were run against `tests/data/20260427.ezd` at the
revision above. No production source was changed and the probe was not
retained.

The first probe inspected the converter result before STEP transfer:

- the shape had 10 unique edges and all 10 lacked a 3D curve;
- the two shared edges each had two pcurves;
- at the same normalized parameter, the maximum distance between the two
  lifted pcurves was `2.33215` model units;
- comparing the first lifted pcurve with the second at reversed normalized
  parameter gave a maximum distance of exactly `0` in the probe;
- `BuildCurves3d(shape, 1e-5)` followed by forced `SameParameter` did not fix
  that representation error: exact `BRepCheck` remained invalid with four
  `BRepCheck_InvalidSameParameterFlag` reports (each bad shared edge is checked
  in both face contexts).

This isolates the direction bug. The original second pcurve comes from the
oppositely directed half-edge, but `addPCurveToEdge` stores it directly on the
natural base edge. The caller then returns `existingEdge.Reversed()` for the
face occurrence; that does not retroactively reverse the stored geometric
representation. [Current converter, lines
263–279](../../src/EzDesignToOCCTConverter.cxx#L263-L279)
[Current `addPCurveToEdge`, lines
378–446](../../src/EzDesignToOCCTConverter.cxx#L378-L446)

The second probe applied the minimal corrective order:

1. reverse the second pcurve into the base edge's natural parameter direction;
2. `BRepLib::BuildCurves3d(shape, 1e-5)`;
3. `BRepLib::SameParameter(shape, 1e-5, Standard_True)`.

Exact pre-export `BRepCheck` then changed from invalid to valid. The generated
STEP changed from degree-1 fallback geometry to cubic B-spline 3D curves, the
round-tripped shape remained exact-valid, and its maximum imported edge/vertex
tolerance was `1e-7`.

Separately, running the unmodified executable:

```sh
build/ezd2step tests/data/20260427.ezd /tmp/20260427.step
```

and inspecting the STEP entities showed 12 translated edge occurrences using
the degree-1, 21-sample fallback. After reimport, exact `BRepCheck` nevertheless
passed. That is an important warning: STEP round-trip validity proves the
written 3D curves and imported pcurves are mutually acceptable; it does not
prove they preserve the source pcurve geometry. The temporary STEP file was
not retained.

Together these experiments show both defects on a checked-in fixture: shared
pcurve sense is inconsistent before export, and the writer hides the missing
3D curves by emitting its coarse fallback rather than the tolerance-driven
`BuildCurves3d` result.

## Minimal closed-loop validation

Use one deliberately small fixture: two non-planar B-spline faces sharing one
visibly curved edge, plus three simple edges per face to close each loop. Give
the two sides different UV parameterizations of the same spatial curve. This
single fixture detects the exact failures of interest without needing a large
production model.

Add a second fixture only if periodic surfaces are supported: one face with a
seam edge occurring twice, with two pcurves separated by one surface period.

For each fixture, use one explicit model-space acceptance tolerance `eps`; do
not derive it from tolerances already stored in the produced shape.

### Before writing STEP

1. Require one topological edge for the shared EzDesign `edge_id`, used with
   opposite contextual orientations in its two face wires.
2. Before attachment, transform each half-edge pcurve into the natural
   direction of that shared topological edge. For the opposite half-edge this
   means reversing its 2D curve/parameterization; reversing only the returned
   `TopoDS_Edge` occurrence is not enough.
3. Require a pcurve in each face context. For a seam require
   `BRep_Tool::IsClosed(edge, face)` and ensure the forward/reversed contextual
   calls return the two intended pcurves.
4. Evaluate source `S_A(P_A(t))` and `S_B(P_B(t))` at endpoints and enough
   interior points; require their bidirectional spatial deviation to be
   `<= eps`. This catches bad EzDesign input before choosing either side as
   the 3D master.
5. Run `BRepLib::BuildCurves3d(shape, eps)` and require success.
6. Run `BRepLib::SameParameter(shape, eps, Standard_True)`.
7. Require `BRepCheck_Analyzer(shape, Standard_True, Standard_False,
   Standard_True).IsValid()`.
8. For every edge, require a non-null `BRep_Tool::Curve`; for every edge/face
   occurrence require a non-null `CurveOnSurface`. Use
   `ShapeAnalysis_Edge::CheckSameParameter(edge, maxDeviation, 101)` and fail
   if `maxDeviation > eps`, regardless of the stored edge tolerance.
9. For every face wire run `ShapeAnalysis_Wire` checks for edge curves,
   connectedness, closure, and self-intersection. Also assert expected counts
   of unique faces/edges/vertices and free edges.

### After writing and reading STEP

1. Require `ReadFile == IFSelect_RetDone`, `NbRootsForTransfer() > 0`,
   `TransferRoots() > 0`, and non-null `OneShape()`.
2. Print both load and transfer diagnostics with `PrintCheckLoad` and
   `PrintCheckTransfer`; fail on transfer failures rather than only parse
   failures. [OCCT 7.9.3 `STEPControl_Reader.hxx`, lines
   70–205](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/STEPControl/STEPControl_Reader.hxx#L70-L205)
3. Repeat pre-export checks 7–9 on the imported shape.
4. Compare imported boundaries against the source reference, not merely against
   the imported pcurves. For the unique curved test edge, sample the source
   lifted pcurve and project each point to the imported 3D edge; sample the
   imported 3D curve and find distance to the source curve-on-surface. Require
   the maximum in both directions to be `<= eps`. This catches a STEP file that
   is internally self-consistent but consistently wrong.
5. Require topology preservation: same unique face/edge/vertex counts, same
   number of edge uses per face, no unexpected free edges, and the seam still
   has two pcurves.

One round-trip test with those assertions is enough to prevent the present
failure. A screenshot or successful import is not an oracle; explicit spatial
deviation is.

## DRAW diagnostic loop

With an OCCT DRAWEXE build that includes the standard modeling, STEP, shape
analysis, and Boolean test plugins:

```tcl
pload MODELING XDE

# If `src` is already a DRAW shape, inspect the intended pre-export repair.
build3d src 1.0e-5
fsameparameter src 1.0e-5
checkshape src -exact
checkcurveonsurf src
comptol src 1001 src_tol

stepwrite a src /tmp/edge-loop.step
testreadstep /tmp/edge-loop.step rt
checkshape rt -exact
checkcurveonsurf rt
comptol rt 1001 rt_tol
bopcheck rt 5
```

`build3d` calls `BRepLib::BuildCurves3d`; `fsameparameter` forces
`BRepLib::SameParameter`; `checkshape -exact` selects exact curve-on-surface
checking; `comptol` reports sampled real 3D/pcurve deviation and its ratio to
stored tolerance; `checkcurveonsurf` reports pairs whose actual deviation
exceeds stored edge tolerance; and `bopcheck ... 5` checks through face/face
self-interferences. [OCCT 7.9.3 DRAW `build3d`, lines
2040–2082](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepTest/BRepTest_CurveCommands.cxx#L2040-L2082)
[OCCT 7.9.3 DRAW `fsameparameter`, lines
409–449](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepTest/BRepTest_BasicCommands.cxx#L409-L449)
[OCCT 7.9.3 DRAW `checkshape`, lines
921–1057](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BRepTest/BRepTest_CheckCommands.cxx#L921-L1057)
[OCCT 7.9.3 DRAW shape-analysis commands, lines
837–941 and 1485–1512](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/SWDRAW/SWDRAW_ShapeAnalysis.cxx#L837-L941)
[OCCT 7.9.3 DRAW STEP commands, lines
293–363 and 409–535](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/XSDRAWSTEP/XSDRAWSTEP.cxx#L293-L535)
[OCCT 7.9.3 DRAW Boolean checks, lines
59–93 and 166–269](https://github.com/Open-Cascade-SAS/OCCT/blob/V7_9_3/src/BOPTest/BOPTest_CheckCommands.cxx#L59-L269)

The numeric `eps` gate still belongs in the automated C++ test: DRAW's
`checkshape`, `checkcurveonsurf`, and `comptol` primarily judge or report
against tolerances stored in the model, which may already be too permissive.

## Ranked likely causes in this exporter

1. **Confirmed on the fixture:** the second face pcurve is stored in the
   opposite sense from the base edge. This makes the pre-export B-rep
   SameParameter-invalid even after building 3D curves.
2. **Confirmed on the fixture:** no prebuilt 3D curves, causing the STEP
   writer's 21-segment fallback. This visually matches faceted/jagged curved
   boundaries.
3. **Likely on periodic surfaces:** seam edges retain only one pcurve because
   the second encounter is mistaken for an already-complete edge.
4. **Possible on other inputs:** face A and face B pcurves do not lift to the
   same spatial curve. `BuildCurves3d` will choose the first, so this must be
   checked before construction and again afterward.
5. **Possible:** parameter ranges or other edge orientations disagree. Forced
   SameParameter plus contextual wire checks expose these.
6. **Masking factor:** enlarged stored tolerances can cause topology checkers
   to accept visible errors; always compare measured deviation with the fixed
   export tolerance.

The minimal correction is not a new custom edge approximation algorithm.
Reverse the second pcurve into the base edge's natural sense, then use OCCT's
existing tolerance-driven 3D builder, force parameter consistency, and make
failure of the validation loop fatal before STEP transfer.

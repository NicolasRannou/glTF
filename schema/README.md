This doc can be used to create the glTF spec.  Many things here have not been fully discussed and are subject to change.

* <a href="#designprinciples">Design Principles</a>
* <a href="#comparison">Comparison between COLLADA and glTF</a>
* <a href="#assetvalidation">Asset Validation</a>
* <a href="#schema">Schema</a> - by category
   * Camera
      * <a href="#camera">`camera`</a>
      * <a href="#orthographic">`orthographic`</a>
      * <a href="#perspective">`perspective`</a>
   * Metadata
      * <a href="#asset">`asset`</a>
      * <a href="#geographicLocation">`geographicLocation`</a>
   * Shading
      * <a href="#image">`image`</a>
      * <a href="#material">`material`</a>
      * <a href="#parameters">`parameters`</a>
      * <a href="#pass">`pass`</a>
      * <a href="#program">`program`</a>   
      * <a href="#shader">`shader`</a>
      * <a href="#states">`states`</a>
      * <a href="#technique">`technique`</a>
      * <a href="#uniform">`uniform`</a>
   * Geometry
      * <a href="#accessor">`accessor`</a>
      * <a href="#attribute">`attribute`</a>
      * <a href="#buffer">`buffer`</a>
      * <a href="#indices">`indices`</a>
      * <a href="#mesh">`mesh`</a>
      * <a href="#node">`node`</a>
      * <a href="#primitive">`primitive`</a>   
* <a href="#schema">Schema</a> - alphabetical
   * <a href="#accessor">`accessor`</a>
   * <a href="#asset">`asset`</a>
   * <a href="#attribute">`attribute`</a>
   * <a href="#buffer">`buffer`</a>
   * <a href="#camera">`camera`</a>
   * <a href="#geographicLocation">`geographicLocation`</a>
   * <a href="#image">`image`</a>
   * <a href="#indices">`indices`</a>
   * <a href="#material">`material`</a>
   * <a href="#mesh">`mesh`</a>
   * <a href="#node">`node`</a>
   * <a href="#orthographic">`orthographic`</a>
   * <a href="#parameters">`parameters`</a>
   * <a href="#pass">`pass`</a>
   * <a href="#perspective">`perspective`</a>
   * <a href="#primitive">`primitive`</a>   
   * <a href="#program">`program`</a>   
   * <a href="#shader">`shader`</a>
   * <a href="#states">`states`</a>
   * <a href="#technique">`technique`</a>
   * <a href="#uniform">`uniform`</a>
* <a href="#references">References</a>
* <a href="#acknowledgments">Acknowledgments</a>

<!-- ----------------------------------------------------------------------- -->
<a name="designgprinciples">
# Design Principles

_TODO: This section was aggregated from glTF slides, wiki pages, etc.  It still needs more details found in those places and to be flushed out._
_TODO: Include great figures from [glTF Architecture and Schema](https://docs.google.com/file/d/0B4owFPtY81iUeUtVT3ZteThJYnM/edit?usp=sharing) and [glTF Ecosystem](https://docs.google.com/file/d/0B4owFPtY81iUT2pDdWR2Q0I2QTQ/edit?usp=sharing) slides._

* glTF is a runtime format; not an interchange.  Its primary use case is rendering.
* glTF strives to map well to OpenGL, OpenGL ES, and WebGL.  To make it useful, glTF has abstractions beyond the GL APIs such as a node hierarchy, materials, and animations.
* To ease adoption, glTF strives to be easy and efficient to load and render.
   * glTF uses JSON for the scene graph, minimizes complicated indirection, and is supported by a JavaScript loader library.
   * glTF uses binary blobs for geometry and textures map directly to GL buffers and textures with no or a minimum amount of application processing, e.g., if geometry compression is used.
   * An open-source asset pipeline converts existing COLLADA assets to glTF and does the heavy lifting (code in progress):
      * Triangulates polygons into triangles.
      * Unifies indices, creating one index per vertex, not per attribute.
      * Splits meshes so indices fit within `UNSIGNED_INT`.
      * Generates shaders and metadata from the COLLADA Common Profile.  Applications can use the glTF shaders or build their own from the metadata.
      * Images are converted to a format supported by browsers, e.g., .jpg or .png.
   * TODO: optimization pipeline.
* glTF strives to keep the asset simple, e.g., negotiate via a REST API instead of in the application, e.g., instead of storing multiple `technique` objects in a glTF asset, the asset only stores one, which can be selected as part of a HTTP request.
* When a tradeoff needs to be made, pain is put on the asset pipeline, e.g., the COLLADA to glTF converter, not the application.  Examples:
   * glTF does not support polygons.  Polygons are triangulated as part of the pipeline.
   * glTF only contains one <a href="#asset">`asset`</a> property.  If a COLLADA asset has several `asset` elements, the convert must handle it, so the applicatio does not have to.
* Just because COLLADA or WebGL has a feature, doesn't mean glTF does.  Examples:
   * All attributes must be backed by buffers, i.e., no [`vertexAttrib`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glVertexAttrib.xml).
   * COLLADA has physics, glTF does not.
* glTF is not part of COLLADA, e.g., it is not a COLLADA profile.  It is its own specification with many designs borrowed from COLLADA and simplified.

<!-- ----------------------------------------------------------------------- -->
<a name="comparison">
# Comparison between COLLADA and glTF

_TODO: Some of this info is generic and does not need to be in a comparasion with COLLADA.  The current organization will confuse users not familiar with COLLADA._

## Naming

COLLADA uses underscores `like_this`; glTF uses [camel case](http://en.wikipedia.org/wiki/CamelCase) `likeThis`.  Camel case is a more common naming convention in JSON and WebGL.

## Angles

COLLADA uses degrees for angles.  To better match GLSL and most math libraries, glTF uses radians.

## id and name

COLLADA `id` attributes map to the name of an object, and the COLLADA `name` attribute maps to the `name` property of an object, for example:
```
<node id="nodeId" name="nodeName"> 
</node>
``` 
becomes
```javascript
"nodeId" : {
    "name" : "nodeName"
}
```

The `name` property is the user-facing name that the engine uses to access parts of the asset, and the object name is used internally to link parts of the asset, e.g., reference a `mesh` from a `node`. 

In glTF, objects that are commonly accessed from an engine, including resource objects (buffers, textures, and shaders), have optional `name` properties.  These objects are:

   * <a href="#buffer">`buffer`</a>
   * <a href="#camera">`camera`</a>
   * <a href="#image">`image`</a>
   * <a href="#material">`material`</a>
   * <a href="#mesh">`mesh`</a>
   * <a href="#node">`node`</a>
   * <a href="#shader">`shader`</a>

What's in COLLADA, but not in this version of glTF
* No `profile` or `technique`.  Instead, this can be negotiated via a REST API.  (platform, product name, etc.)

_TODO_
* _Does the converter preserve COLLADA `id` attributes?_
* _Converter needs to preserve COLLADA `name` attributes for camera, etc. above._
* _Support `sid`, especially for reusing animations._

## extra

glTF allows application-specific metadata on every object using an `extra` property.  This is similar to `extra` in COLLADA, Pages 5-35 to 5-36, except `technique` is not required.

_TODO_
* _COLLADA2JSON_
   * _Needs to output `extra` objects_.
* _Loader_
   * _Needs to pass through `extra` objects_.
   * _Should fill out all optional properties so that the user receives complete objects with default values, e.g., render state._

Alternatively, glTF could allow application-specific properties anywhere, but this has the potential to break backwards compatibility in future versions, e.g., if an asset uses a property name that is then used in a future version of glTF.  Therefore, glTF does not allow additional properties on any objects, except `extra`.

<!-- ----------------------------------------------------------------------- -->
<a name="assetvalidation">
# Asset Validation

The glTF schema is written using [JSON Schema 03](http://tools.ietf.org/html/draft-zyp-json-schema-03).

An asset can be validated against the schema using the glTF Validator (based on [JSV](https://github.com/garycourt/JSV)).  Setup a local webserver to point to the `schema` directory, and browse to index.html.  For example:
```
http://localhost/gltf/
```

There are a few query parameters.

Use `schema` to provide the schema to validate against:
```
http://localhost/gltf/?schema=states.schema.json
```

Use `json` to provide the asset's JSON to validate:
```
http://localhost/gltf/?json=examples/states/translucency.json
```

Use `validate` to immediately validate the json with the schema:
```
http://localhost/gltf/?validate=true
```

The most common use is something like:
```
http://localhost/gltf/?schema=states.schema.json&json=examples/states/translucency.json&validate=true
```

Also, JSON in general can be valdiated with [JSONLint](http://jsonlint.com/), which can be useful for validating the glTF schema itself.

_TODO: JSON Schema is not rigorous like XML.  We need custom tools to fully validate an asset._

<!-- ----------------------------------------------------------------------- -->
<a name="schema">
# Schema

<!-- ----------------------------------------------------------------------- -->
<a name="accessor">
## `accessor`

* Schema: [accessor.schema.json](accessor.schema.json)
* Example: [accessor.json](examples/accessor/accessor.json) - every property with example values.

### Details

* To satisfy Section 6.3 (Buffer Offset and Stride Requirements) of the WebGL 1.0 Spec, `byteOffset` and `byteStride` must be a multiple of the size of `componentType`.
* To satisfy Section 6.9 (Vertex Attribute Data Stride) of the WebGL 1.0 Spec, `byteStride` cannot exceed 255.

### Related GL Functions

* [`vertexAttribPointer`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glVertexAttribPointer.xml)
* [`enableVertexAttribArray`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glEnableVertexAttribArray.xml)
* [`disableVertexAttribArray`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glEnableVertexAttribArray.xml)

### Differences from COLLADA

* Renamed `offset` to `byteOffset`.
* Renamed `stride` to `byteStride`.
   
### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

* `accessor`. Pages 5-5 to 5-10.

### _Open Questions_

* _Schema_
   * I personally want a `center` and `radius`, not `min` and `max`.  However, `min` and `max` is more general.  Perhaps add `center` and `radius` to `primitive`?
* _COLLADA2JSON_
   * _Move `semantic` from `primitives` to `accessor`.  What is the benefit of decoupling them?_
   * _Rename `VERTEX` semantic to `POSITION`._
   * _Add checking for Sections 6.3 and 6.9 above._
   * _What other well-known semantics should there be?  App-specific semantics are allowed, of course._
   
<!-- ----------------------------------------------------------------------- -->
<a name="asset">
## `asset`

* Schema: [asset.schema.json](asset.schema.json)
* Examples:
   * [asset.json](examples/glTF/asset.json) - bare glTF model with an asset property.
   * [asset.json](examples/asset/asset.json) - every property with example values.

### Details

The design of `asset` in COLLADA is focused on asset interchange for assets potentially authored by multiple artists.  In glTF, `asset` is used to credit all artists for display purposes.

### Related GL Functions

None.

### Differences from COLLADA

* `asset` is only defined on the root glTF property; in COLLADA, it is available as a child of many elements.  In practice, it is usually only a child of the `COLLADA` root node.
* `asset` just contains `copyright` that can be used to credit the model author(s), and `geographicLocation`.  If an application needs more information, it can be stored in the `extra` property.

### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

* `asset`. Pages 5-17 to 5-19
* `contributor`. Pages 5-27 to 5-28.
* `geographic_location`. Pages 5-40 to 5-41.

### _Open Questions_

* _Schema_
   * _Need `unit` property?  Or always meters and apply a scale matrix to the root?_
   * _Also need url for copyright image?_
   * _Need to figure out_ `up_axis`.
* _COLLADA2JSON_
   * _Create `asset.copyright` from COLLADA author and copyright?_
   * _Convert COLLADA geographic_location.  glTF uses radians, not degrees._
   * _Convert kml/kmz location to geographic_location.  Convert kml/kmz orientation and scale to a transform._

<!-- ----------------------------------------------------------------------- -->
<a name="attribute">
## `attribute`

* Schema: [attribute.schema.json](attribute.schema.json)
* Example: [attribute.json](examples/attribute/attribute.json) - every property with example values.

### Details

_TODO_

### Related GL Functions

* [`bindAttribLocation`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glBindAttribLocation.xml)
* [`getAttribLocation`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glGetAttribLocation.xml)
* [`getProgramParameter`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glGetProgramiv.xml) with `ACTIVE_ATTRIBUTES`

### Differences from COLLADA

_TODO_

### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

_TODO_

### _Open Questions_

* _Schema_
   * Create list of built-in semantics. [#45](https://github.com/KhronosGroup/collada2json/issues/45).
   * Create list of allowed types.
* _COLLADA2JSON_
   * 

<!-- ----------------------------------------------------------------------- -->
<a name="buffer">
## `buffer`

* Schema: [buffer.schema.json](buffer.schema.json)
* Examples:
   * [buffers.json](examples/glTF/buffers.json) - bare glTF model with a buffer.
   * [buffer.json](examples/buffer/buffer.json) - every property with example values.

### Details

Instead of referencing an external binary file, the URL may be a base64 [data URI](https://developer.mozilla.org/en/data_URIs) to facilitate storing all model assets in a single .json for easy deployment, drag and drop, etc.  This can be negotiated via a REST API.

_TODO: Even though data URIs are part of the [spec](https://dvcs.w3.org/hg/xhr/raw-file/tip/Overview.html#data:-urls-and-http), they do not work with `XMLHttpRequest` in Chrome. We need to get this [bug](http://code.google.com/p/chromium/issues/detail?id=46806) fixed._

### Related GL Functions

* `createBuffer` / [`genBuffers`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glGenBuffers.xml)
* `deleteBuffer` / [`deleteBuffers`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glDeleteBuffers.xml)
* [`bindBuffer`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glBindBuffer.xml)
* [`bufferData`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glBufferData.xml)
* [`bufferSubData`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glBufferSubData.xml)

### Differences from COLLADA

* Vertices and indices are stored in binary, not XML.
* From the buffer's perspective, vertices and indices are untyped unlike `float_array`, etc. in COLLADA.  The `accessor` specifies the type later for the subset it references.
* glTF does not support `float_array` attributes `digits` and `magnitude`.  In glTF, all floats are stored in IEEE-754 32-bit floating point.
* glTF does not support `int_array` attributes `minInclusive` or `maxInclusive`.  The `accessor` specifies the type, e.g., unsigned short or unsigned int, later for the subset it references.
* glTF does not support `IDREF_array` (5-44), `Name_array` (5-94 to 5-95), or `SIDREF_array` (5-130).
* Also see <a href="#mesh">`mesh`</a>.

### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

* `geometry`.  Pages 5-42 to 5-43.
* `mesh`. Pages 5-89 to 5-91.
* `source`. Pages 5-137 to  5-138.
* `bool_array`. Page 5-20.
* `float_array`. Page 5-37.
* `int_array`. Page 5-69.

### _Open Questions_

* _Schema_
   * Why does current version include `type`: "ArrayBuffer"`?
   * Separate buffers for indices and vertices since WebGL treats them separately?
* _COLLADA2JSON_
   * Convert `bool_array` to `0.0` or `1.0`?
   * Use `int_array` attributes `minInclusive` or `maxInclusive` to determine WebGL int datatype?
   
<!-- ----------------------------------------------------------------------- -->
<a name="camera">
## `camera`

* Schema:
   * [camera.schema.json](camera.schema.json)
   * [perspective.schema.json](perspective.schema.json)
   * [orthographic.schema.json](orthographic.schema.json).
* Examples
   * [cameras.json](examples/glTF/cameras.json) - bare glTF model with two cameras.
   * [camera.json](examples/camera/camera.json) - every property with example values.
   * [orthographic.json](examples/orthographic/orthographic.json) - orthographic camera using all properties.
   * [perspective.json](examples/perspective/perspective.json) - perspective camera using all properties.

### Details

For `orthographic`, the following properties are required:
* `xmag` or `ymag`
   
For `perspective`, the following properties are required:
* `aspectRatio`
* `yfov`

### Related GL Functions

See <a href="#orthographic">`orthographic`</a> and <a href="#perspective">`perspective`</a>.

### Differences from COLLADA

In order to better map to OpenGL, OpenGL ES, and WebGL, glTF differs from COLLADA in the following ways:

* No `imager`.  COLLADA Pages 5-45 to 5-46.
* No `optics`.  COLLADA Pages 5-100 to 5-101.  `orthographic` and `perspective` are directly part of the camera.
   * A glTF camera represents a single camera; it doesn't have techniques.  This can be negotiated via a REST API.
* In COLLADA, both `orthographic` and `perspective` allow several different options for required properties.  glTF provides only one set of required properties for `orthographic` and `perspective` cameras, which map most commonly to computing orthographic and perspective projection matrices.  See above.

### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

* `camera`.  Pages 5-21 to 5-22.
* `orthographic`.  Pages 5-102 to 5-103.
* `perspective`.  Pages 5-108 to 5-109.

### _Open Questions_

* _COLLADA2JSON_
   * _Loader and writer need to be updated to reflect the new organization and required properties, not all COLLADA properties._
   * _`yfov` is degrees; it should be radians since this is a final-stage format._

<!-- ----------------------------------------------------------------------- -->
<a name="geographicLocation">
## `geographicLocation`

* Schema: [geographicLocation.schema.json](geographicLocation.schema.json)
* Example: [geographicLocation.json](examples/geographicLocation/geographicLocation.json) - every property with example values.

### Related GL Functions

None.

### Details

_TODO_

### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

_TODO_

### _Open Questions_

_TODO_

<!-- ----------------------------------------------------------------------- -->
<a name="image">
## `image`

* Schema: [image.schema.json](image.schema.json)
* Examples
   * [images.json](examples/glTF/images.json) - bare glTF model with two images.
   * [image.json](examples/image/image.json) - every property with example values.

### Details

For compatibility with modern web browsers, the following image formats are supported: `.jpg`, `.png`, `.bmp`, and `.gif`.  The URL may be a [data URI](https://developer.mozilla.org/en/data_URIs) to facilitate storing all model assets in a single .json for easy deployment, drag and drop, etc.  This can be negotiated via a REST API.

### Related GL Functions

* `createTexture` / [`genTextures`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glGenTextures.xml)
* `deleteTexture` / [`deleteTextures`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glDeleteTextures.xml)
* [`generateMipmap`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glGenerateMipmap.xml)
* [`bindTexture`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glBindTexture.xml)
* [`texImage2D`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glTexImage2D.xml)
* [`texSubImage2D`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glTexSubImage2D.xml)

### Differences from COLLADA

In order to better map to OpenGL, OpenGL ES, and WebGL, glTF differs from COLLADA in the following ways:
* Renamed `mips_generate` to `generateMipmap`.

glTF 1.0 does not support:
* `image`: `renderable`, `create_2d`, `create_3d`, and `create_cube`.
* `init_from`: `array_index`, `mip_index`, `depth`, and `face`.

### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

* `library_images`.  Page 8-75.
* `image`.  Page 8-58 to 8-60.
* `init_from`.  Pages 8-62 to 8-63.

### _Open Questions_

* _Schema_
   * _Support precomputed mips?_
   * _Allow blank texture as a render target and other ColladaFX features on 8-59?_
   * _Consider support for `UNPACK_FLIP_Y_WEBGL`, `UNPACK_PREMULTIPLY_ALPHA_WEBGL`, and `UNPACK_COLORSPACE_CONVERSION_WEBGL`._

* _COLLADA2JSON_
   * _Add support for `generateMipmap`_
   * _Image conversion to `.jpg` or `.png`_
   * _Add option for data uri_

<!-- ----------------------------------------------------------------------- -->
<a name="indices">
## `indices`

* Schema: [indices.schema.json](indices.schema.json)
* Example: [indices.json](examples/indices/indices.json) - every property with example values.

### Details

_TODO_

### Related GL Functions

* [`drawElements`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glDrawElements.xml)

### Differences from COLLADA

_TODO_

### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

_TODO_

### _Open Questions_

_TODO_

<!-- ----------------------------------------------------------------------- -->
<a name="material">
## `material`

* Schema: [material.schema.json](material.schema.json)
* Examples
   * [materials.json](examples/glTF/materials.json) - bare glTF model with two images.
   * [material.json](examples/material/material.json) - every property with example values.

### Details

_TODO_

### Related GL Functions

See <a name="parameters">`parameters`</a>.

### Differences from COLLADA

_TODO_

### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

_TODO_

### _Open Questions_

* _Schema_
   * _Write schema for `parameters`._
* _COLLADA2JSON_
   * _Remove `techniques` since a material only has one.  Instead, this can be negotiated via a REST API._
   
<!-- ----------------------------------------------------------------------- -->
<a name="mesh">
## `mesh`

* Schema: [mesh.schema.json](mesh.schema.json)
* Example:  [mesh.json](examples/mesh/mesh.json) - every property with example values.

### Details

_TODO_

### Related GL Functions

See <a href="#accessors">`accessors`</a> and <a href="#primitives">`primitives`</a>.

### Differences from COLLADA

* glTF does not support `spline`, `convex_mesh` (physics), or `brep` (B-Rep).
* No `technique_common` or `technique`.  Instead, this can be negotiated via a REST API.
* Also see <a href="#buffer">`buffer`</a>.

### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

_TODO_

### _Open Questions_

* _Schema_
   * Is a `byteStride` of zero tightly packed like WebGL?  Yes?
* _COLLADA2JSON_
   * Rename `elementType` to `componentType` to better match WebGL.
   * Rename `elementsPerValue` to `componentsPerAttribute`.
   * Rename `componentType` `Float32` (and others?) to `FLOAT` to better match WebGL.
   * Add support for `normalized`.
   * Long term, we should convert `spline`.

<!-- ----------------------------------------------------------------------- -->
<a name="node">
## `node`

* Schema: [node.schema.json](node.schema.json)
* Examples
   * [nodes.json](examples/glTF/nodes.json) - bare glTF model with two nodes.
   * [meshes.json](examples/node/meshes.json) - an interior node with a mesh (and children).
   * [meshes_leaf.json](examples/node/meshes_leaf.json) - a leaf node with a mesh.

### Details

_TODO_

### Related GL Functions

* [`uniformMatrix[234]fv`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glUniform.xml)

Also see <a href="#mesh">`accessors`</a> sub properties See <a href="#accessors">`accessors`</a> and <a href="#primitives">`primitives`</a>.

### Differences from COLLADA

In order to better map to OpenGL, OpenGL ES, and WebGL, glTF differs from COLLADA in the following ways:
* Only a single transform is supported, and it must be a 4x4 matrix.  COLLADA transformation elements such as `lookat` and `rotate` must be converted to a 4x4 matrix.  If several COLLADA transformation elements are used, they must be concatenated into a single 4x4 matrix.

### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

* `node`.  Pages 5-98 to 5-99.

### _Open Questions_

* _Schema_
   * _instance_camera_ - keep camera as a node in the scene graph?  What are the use cases?  Shadow mapping?_
   * _instance_controller_
   * _instance_light_
   * Allow only one? _meshes_, _camera_, or _lights_.
   * _Animations for all COLLADA transformation elements is going to be hard.  Need to scope it right._
* _COLLADA2JSON_
   * _Convert all transformation elements to 4x4 matrix.  Do we already?_

<!-- ----------------------------------------------------------------------- -->
<a name="orthographic">
## `orthographic`

* Schema: [orthographic.schema.json](orthographic.schema.json)
* Example: [orthographic.json](examples/orthographic/orthographic.json) - every property with example values.

### Details

_TODO_

### Related GL Functions

* [`uniformMatrix4fv`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glUniform.xml)

### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

_TODO_

### _Open Questions_

_TODO_

<!-- ----------------------------------------------------------------------- -->
<a name="parameters">
## `parameters`

* Schema: [parameters.schema.json](parameters.schema.json)
* Example: [parameters.json](examples/parameters/parameters.json) - every property with example values.

### Details

_TODO_

### Related GL Functions

* [`activeTexture`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glActiveTexture.xml)
* [`bindTexture`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glBindTexture.xml)
* [`texParameterf`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glTexParameter.xml)
* [`texParameteri`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glTexParameter.xml)
* [`uniform[1234][fi]`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glUniform.xml)
* [`uniform[1234][fi]v`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glUniform.xml)
* [`uniformMatrix[234]fv`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glUniform.xml)

### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

_TODO_

### _Open Questions_

_TODO_

<!-- ----------------------------------------------------------------------- -->
<a name="pass">
## `pass`

* Schema: [pass.schema.json](pass.schema.json)
* Examples: [pass.json](examples/pass/pass.json) - every property with example values.

### Details

_TODO_

### Related GL Functions

See <a href="#states">`states`</a> or <a href="#program">`program`</a>.

### Differences from COLLADA

_TODO_

### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

_TODO_

### _Open Questions_

* _Schema_
   * 
* _COLLADA2JSON_
   *

<!-- ----------------------------------------------------------------------- -->
<a name="perspective">
## `perspective`

* Schema: [perspective.schema.json](perspective.schema.json)
* Example: [perspective.json](examples/perspective/perspective.json) - every property with example values.

### Details

_TODO_

### Related GL Functions

* [`uniformMatrix4fv`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glUniform.xml)

### Differences from COLLADA

_TODO_

### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

_TODO_

### _Open Questions_

_TODO_
   
<!-- ----------------------------------------------------------------------- -->
<a name="primitive">
## `primitive`

* Schema: [primitive.schema.json](primitive.schema.json)
* Example: [primitive.json](examples/primitive/primitive.json) - every property with example values.

### Details

* To satisfy Section 6.3 (Buffer Offset and Stride Requirements) of the WebGL 1.0 Spec, `byteOffset` must be a multiple of the size of `type`.

### Related GL Functions

* [`drawElements`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glDrawElements.xml)

Also see <a href="#material">`material`</a> and <a href="#technique">`technique`</a> subproperty <a href="#parameters">`parameters`</a>.

### Differences from COLLADA

TODO
   
### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

* 

### _Open Questions_

* _Schema_
   * `indices` `type` could also be `UNSIGNED_BYTE`, but I don't think anyone uses it.
* _COLLADA2JSON_
   * Renamed `length` to `count` to better match WebGL.
   * _Add checking for Section 6.3 above._

<!-- ----------------------------------------------------------------------- -->
<a name="program">
## `program`

* Schema: [program.schema.json](program.schema.json)
* Examples: [program.json](examples/program/program.json) - every property with example values.

### Details

_TODO_

### Related GL Functions

* [`createProgram`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glCreateProgram.xml)
* [`deleteProgram`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glDeleteProgram.xml)
* [`attachShader`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glAttachShader.xml)
* [`linkProgram`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glLinkProgram.xml)
* [`useProgram`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glUseProgram.xml)
* [`validateProgram`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glValidateProgram.xml)

Also see <a href="#shader">`shader`</a> and subproperties <a href="#attributes">`attributes`</a> and <a href="#uniforms">`uniforms`</a>.

### Differences from COLLADA

_TODO_

### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

_TODO_

### _Open Questions_

* _Schema_
   * 
* _COLLADA2JSON_
   * Renamed `x-shader/x-fragment` to `fragmentShader` to better match WebGL
   * Renamed `x-shader/x-vertex` to `vertexShader` to better match WebGL

<!-- ----------------------------------------------------------------------- -->
<a name="shader">
## `shader`

* Schema: [shader.schema.json](shader.schema.json)
* Examples
   * [shaders.json](examples/glTF/shaders.json) - bare glTF model with two shaders.
   * [shader.json](examples/shader/shader.json) - every property with example values.
   * [dataUri.json](examples/shader/dataUri.json) - using a data URI for a shader source.

### Details

GLSL source can be in external plain-text .glsl files.  The URL may also be a plain-text [data URI](https://developer.mozilla.org/en/data_URIs) to facilitate storing all model assets in a single .json for easy deployment, drag and drop, etc.  Using a data URI reduces the number of requests for a single model.  However, if several models are loaded that use the same shader, using separate .glsl files may be better due to HTTP caching.  This can be negotiated via a REST API.

_TODO: Even though data URIs are part of the [spec](https://dvcs.w3.org/hg/xhr/raw-file/tip/Overview.html#data:-urls-and-http), they do not work with `XMLHttpRequest` in Chrome. We need to get this [bug](http://code.google.com/p/chromium/issues/detail?id=46806) fixed._

### Related GL Functions

* [`createShader`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glCreateShader.xml)
* [`compileShader`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glCompileShader.xml)
* [`deleteShader`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glDeleteShader.xml)
* [`shaderSource`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glShaderSource.xml)

### Differences from COLLADA

_TODO_

### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

_TODO_

### _Open Questions_

* _Schema_
   * _Metadata, e.g., phong, etc._
* _COLLADA2JSON_
   * _Option for data uri._

<!-- ----------------------------------------------------------------------- -->
<a name="states">
## `states`

* Schema: [states.schema.json](states.schema.json)
   * All properties are optional and default to values from the [WebGL 1.0 spec](https://www.khronos.org/registry/webgl/specs/1.0/).
* Examples:
   * [states.json](examples/states/states.json) - every state with its default value.
   * [closed_opaque_object.json](examples/states/closed_opaque_object.json) - typical state for rendering a closed, opaque object: depth testing and backface culling.
   * [depth_test.json](examples/states/depth_test.json) - state with depth testing enabled.
   * [translucency.json](examples/states/translucency.json) - typical state for rendering a translucency object with alpha blending.

### Details

Like COLLADA, glTF includes:
* `blendEquation` and `blendEquationSeparate`
* `both blendFunc` and `blendFuncSeparate`
* `stencilFunc` and `stencilFuncSeparate`
* `stencilOp` and `stencilOpSeparate`

The separate version of a property takes precedence over its counterpart.  For example, if both `blendEquation` and `blendEquationSeparate` are provided, `blendEquationSeparate` takes precedence.

To satisfy Section 6.8 (Stencil Separate Mask and Reference Values) of the WebGL 1.0 Spec, when `stencilFuncSeparate` is used, `ref` and `mask` must be the same for front- and back-facing geometry.
To satisfy Section 6.10 (Viewport Depth Range) of the WebGL 1.0 Spec, `zNear` cannot be greater than `zFar`.

### Differences from COLLADA

Render states are based on the GLES2 profile in [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf), Pages 8-120 to 8-125.  In order to better map to OpenGL, OpenGL ES, and WebGL, glTF differs from COLLADA in the following ways:

* `blendFunc`, renamed
   * `src` to `sfactor`
   * `dst` to `dfactor`
* `blendFuncSeparate`, renamed
   * `src_rgb` to `srcRGB`
   * `dest_rgb` to `dstRGB`
   * `src_alpha` to `srcAlpha`
   * `dest_alpha` to `dstAlpha`
* Changed `colorMask` from an array of four booleans to an object with four properties: `red`, `green`, `blue`, and `alpha`.
* Removed `point_size_enable`; instead, we just assign to `gl_PointSize` in WebGL based on `pointSize`.
* `polygonOffset`
   * Replaced `float2` array with `factor` and `units` properties.
* `scissor` `width` and `height` default to zero, not "When a GL context is first attached to a window, width and height are set to the dimensions of that window."
* Added `sampleCoverageEnable`.
* To satisfy Section 6.8 (Stencil Separate Mask and Reference Values) of the WebGL 1.0 Spec, there is no `stencil_mask_separate`.
   
### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

* `states`. Pages 8-120 to 8-125.

### Related GL Functions

_TODO_

### _Open Questions_

* _Schema_
   * _Do we agree about separate precedence above?_
   * _I don't know anyone who uses `dither_enable` nowadays.  Is this something we want to include?_
   * _Currently, only a `lineWidth` of `1.0` is supported on most browsers on Windows because that is all that ANGLE supports._
   * _Need to look at OpenGL and OpenGL ES, which I think still includes `point_size_enable`._
* _COLLADA2JSON_
   * _Add to loader and writer.  Writer needs to derive state from common profile._

<!-- ----------------------------------------------------------------------- -->
<a name="technique">
## `technique`

* Schema: [technique.schema.json](technique.schema.json)
* Examples
   * [techniques.json](examples/glTF/techniques.json) - bare glTF model with two images.
   * [technique.json](examples/technique/technique.json) - every property with example values.

### Details

_TODO_

### Related GL Functions

See <a name="parameters">`parameters`</a>.

### Differences from COLLADA

_TODO_

### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

_TODO_

### _Open Questions_

* _Schema_
   * _Write schema for `parameters`._
   * _Do we need `type": "program"`?_
* _COLLADA2JSON_
   * _Remove `passes` since we are only supporting a single pass to start?_

<!-- ----------------------------------------------------------------------- -->
<a name="uniform">
## `uniform`

* Schema: [uniform.schema.json](uniform.schema.json)
* Example: [uniform.json](examples/uniform/uniform.json) - every property with example values.

### Details

_TODO_

### Related GL Functions

[`getActiveUniform`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glGetActiveUniform.xml)
[`getUniformLocation`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glGetUniformLocation.xml)
[`getProgramParameter`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glGetProgramiv.xml) with `ACTIVE_UNIFORMS`
* [`uniform[1234][fi]`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glUniform.xml)
* [`uniform[1234][fi]v`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glUniform.xml)
* [`uniformMatrix[234]fv`](http://www.khronos.org/opengles/sdk/docs/man/xhtml/glUniform.xml)

### Differences from COLLADA

_TODO_

### [COLLADA 1.5](http://www.khronos.org/files/collada_spec_1_5.pdf) References

_TODO_

### _Open Questions_

* _Schema_
   * Create list of built-in semantics. [#45](https://github.com/KhronosGroup/collada2json/issues/45).
   * Create list of allowed types.
   * Rename VERTEX semantic to POSITION.
   * Rename `FLOAT_VEC3` `type` to `vec3`, etc.
* _COLLADA2JSON_
   * 
   
<!-- ----------------------------------------------------------------------- -->
<a name="references">
# References

* [COLLADA 1.5 spec](http://www.khronos.org/files/collada_spec_1_5.pdf)
* [WebGL 1.0 spec](https://www.khronos.org/registry/webgl/specs/1.0/)

<!-- ----------------------------------------------------------------------- -->
<a name="acknowledgments">
# Acknowledgments

* Tom Fili, Analytical Graphics, Inc., answers all of Patrick's COLLADA questions.

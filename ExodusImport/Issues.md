As of now:

* It is not possible to toggle simple/complex collision on per-component level, it seems we'll need to duplicate meshes.
* KDop mesh generation fails on objects such as plane. Need to add fallback for situation when nothing got generated.
* Mapping to rigidbody is not 1-to-1, we need a "rigidbody" primtiive with no collision and no tensor to act like one.
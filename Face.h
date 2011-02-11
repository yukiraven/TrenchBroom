//
//  Face.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Observable.h"

extern NSString* const FaceGeometryChanged;
extern NSString* const FaceFlagsChanged;

extern NSString* const TextureOldKey;
extern NSString* const TextureNewKey;

typedef enum {
    XY, XZ, YZ
} EPlaneType;

@class Vector2f;
@class Vector3f;
@class Vector3i;
@class HalfSpace3D;
@class Plane3D;
@class Brush;
@class Matrix4f;
@class PickingHit;
@class Ray3D;

@interface Face : Observable {
    @private
    Brush* brush;
    NSNumber* faceId;
    
	Vector3i* point1;
	Vector3i* point2;
	Vector3i* point3;
	
	NSMutableString* texture;
	int xOffset;
	int yOffset;
	float rotation;
	float xScale;
	float yScale;
    
    Vector3f* norm;
    HalfSpace3D* halfSpace;
    
    int bestAxis;
    Vector3f* texAxisX;
    Vector3f* texAxisY;
    
    // transforms surface coordinates to world coordinates
    Matrix4f* surfaceMatrix;
    Matrix4f* worldMatrix; // inverse of surface matrix
}

- (id)initInBrush:(Brush *)theBrush point1:(Vector3i *)aPoint1 point2:(Vector3i *)aPoint2 point3:(Vector3i *)aPoint3 texture:(NSString *)aTexture;

- (NSNumber *)faceId;
- (Brush *)brush;

- (Vector3i *)point1;
- (Vector3i *)point2;
- (Vector3i *)point3;

- (NSString *)texture;
- (int)xOffset;
- (int)yOffset;
- (float)rotation;
- (float)xScale;
- (float)yScale;

- (Vector3f *)norm;

- (void)setPoint1:(Vector3i *)thePoint1 point2:(Vector3i *)thePoint2 point3:(Vector3i *)thePoint3;
- (void)translateBy:(Vector3i *)theDelta;

- (void)setTexture:(NSString *)name;
- (void)setXOffset:(int)offset;
- (void)setYOffset:(int)offset;
- (void)setRotation:(float)angle;
- (void)setXScale:(float)factor;
- (void)setYScale:(float)factor;
- (void)translateOffsetsX:(int)x y:(int)y;

- (void)texCoords:(Vector2f *)texCoords forVertex:(Vector3f *)vertex;
- (Vector3f *)worldCoordsOf:(Vector3f *)sCoords;
- (Vector3f *)surfaceCoordsOf:(Vector3f *)wCoords;
- (HalfSpace3D *)halfSpace;
- (PickingHit *)pickWithRay:(Ray3D *)theRay;
/*!
    @function
    @abstract   Returns the center of this face.
    @result     The center of this face.
*/
- (Vector3f *)center;

/*!
    @function
    @abstract   Returns this face's vertices in clockwise order.
    @result     An array containing the vertices.
*/
- (NSArray *)vertices;

/*!
    @function
    @abstract   Returns the undo manager for this face.
    @discussion This method simply returns the undo manager of the brush to which this face belongs.
    @result     The undo manager for this face or nil if there is no undo manager.
*/
- (NSUndoManager *)undoManager;

- (BOOL)postNotifications;
@end

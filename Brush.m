//
//  Brush.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Brush.h"
#import "Entity.h"
#import "IdGenerator.h"
#import "Vector3f.h"
#import "Vector3i.h"
#import "Face.h"
#import "HalfSpace3D.h"
#import "VertexData.h"
#import "BoundingBox.h"
#import "Ray3D.h"
#import "PickingHit.h"

NSString* const BrushFaceAdded = @"BrushFaceAdded";
NSString* const BrushFaceRemoved = @"BrushFaceRemoved";

NSString* const FaceKey = @"Face";

@implementation Brush

- (id)init {
    if (self = [super init]) {
        brushId = [[[IdGenerator sharedGenerator] getId] retain];
        faces = [[NSMutableArray alloc] init];
        vertexData = [[VertexData alloc] init];
        
        flatColor[0] = (rand() % 255) / 255.0f;
        flatColor[1] = (rand() % 255) / 255.0f;
        flatColor[2] = (rand() % 255) / 255.0f;
    }
    
    return self;
}

- (id)initInEntity:(Entity *)theEntity {
    if (theEntity == nil)
        [NSException raise:NSInvalidArgumentException format:@"entity must not be nil"];
    
    if (self = [self init]) {
        entity = theEntity; // do not retain
    }
    
    return self;
}

- (void)faceGeometryChanged:(NSNotification *)notification {
    [vertexData release];
    vertexData = nil;
}

- (VertexData *)vertexData {
    if (vertexData == nil) {
        NSMutableArray* droppedFaces = nil;
        vertexData = [[VertexData alloc] initWithFaces:faces droppedFaces:&droppedFaces];
        if (droppedFaces != nil) {
            NSEnumerator* droppedFacesEn = [droppedFaces objectEnumerator];
            Face* droppedFace;
            while ((droppedFace = [droppedFacesEn nextObject])) {
                NSLog(@"Face %@ was cut away", droppedFace);
                [self removeFace:droppedFace];
            }
        }
    }
    
    return vertexData;
}

- (Face *)createFaceWithPoint1:(Vector3i *)point1 point2:(Vector3i *)point2 point3:(Vector3i *)point3 texture:(NSString *)texture {
    Face* face = [[Face alloc] initInBrush:self point1:point1 point2:point2 point3:point3 texture:texture];

    if (![self addFace:face]) {
        [face release];
        return nil;
    }
    
    return [face autorelease];
}

- (BOOL)addFace:(Face *)face {
    NSUndoManager* undoManager = [self undoManager];

    NSMutableArray* droppedFaces = nil;
    if (![[self vertexData] cutWithFace:face droppedFaces:&droppedFaces]) {
        NSLog(@"Brush %@ was cut away by face %@", self, face);
        return NO;
    } else {
        [[undoManager prepareWithInvocationTarget:self] removeFace:face];
    }
    
    if (droppedFaces != nil) {
        NSEnumerator* droppedFacesEn = [droppedFaces objectEnumerator];
        Face* droppedFace;
        while ((droppedFace = [droppedFacesEn nextObject])) {
            NSLog(@"Face %@ was cut away by face %@", droppedFace, face);
            [self removeFace:droppedFace];
        }
    }

    [face addObserver:self selector:@selector(faceGeometryChanged:) name:FaceGeometryChanged];
    [self addForward:FaceGeometryChanged from:face];
    [self addForward:FaceFlagsChanged from:face];

    [faces addObject:face];
    [vertexData release];
    vertexData = nil;
   
    [self notifyObservers:BrushFaceAdded infoObject:face infoKey:FaceKey];
    return YES;
}

- (void)removeFace:(Face *)face {
    NSUndoManager* undoManager = [self undoManager];
    [[undoManager prepareWithInvocationTarget:self] addFace:face];
    
    [face removeObserver:self];
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:face forKey:FaceKey];
    
    [faces removeObject:face];
    [vertexData release];
    vertexData = nil;
    
    [self notifyObservers:BrushFaceRemoved userInfo:userInfo];
}

- (Entity *)entity {
    return entity;
}

- (NSNumber *)brushId {
    return brushId;
}
         
- (NSArray *)faces {
    return faces;
}

- (NSArray *)verticesForFace:(Face *)face {
    if (face == nil)
        [NSException raise:NSInvalidArgumentException format:@"face must not be nil"];

    return [vertexData verticesForFace:face];
}

- (float *)flatColor {
    return flatColor;
}

- (BoundingBox *)bounds {
    return [[self vertexData] bounds];
}

- (Vector3f *)centerOfFace:(Face *)face {
    return [[self vertexData] centerOfFace:face];
}

- (PickingHit *)pickFace:(Ray3D *)theRay; {
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject])) {
        PickingHit* hit = [vertexData pickFace:face withRay:theRay];
        if (hit != nil)
            return hit;
    }
    
    return nil;
}

- (PickingHit *)pickFace:(Face *)theFace withRay:(Ray3D *)theRay {
    return [vertexData pickFace:theFace withRay:theRay];
}

- (void)translateBy:(Vector3i *)theDelta {
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face translateBy:theDelta];
}

- (BOOL)postNotifications {
    return [entity postNotifications];
}

- (NSUndoManager *)undoManager {
    return [entity undoManager];
}

- (void)dealloc {
    NSEnumerator* faceEn = [faces objectEnumerator];
    Face* face;
    while ((face = [faceEn nextObject]))
        [face removeObserver:self];
    
    [brushId release];
    [vertexData release];
    [faces release];
    [super dealloc];
}

@end

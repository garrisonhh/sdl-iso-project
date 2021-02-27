#include <stdlib.h>
#include <stdbool.h>
#include "collision.h"
#include "vector.h"
#include "entity.h"
#include "world.h"
#include "utils.h"

#include <stdio.h>

bool collides(double a, double b, double x) {
	return (a < x) && (x < b);
}

bool collide1d(double startA, double lenA, double startB, double lenB) {
	return collides(startB, startB + lenB, startA)
		|| collides(startB, startB + lenB, startA + lenA)
		|| (fisClose(startA, startB) && fisClose(lenA, lenB));
}

// checks if boxA is inside boxB, but not vice versa
// if boxA surrounds boxB, no collision will be detected
bool bboxCollide(bbox_t boxA, bbox_t boxB) {
	return collide1d(boxA.offset.x, boxA.size.x, boxB.offset.x, boxB.size.x)
		&& collide1d(boxA.offset.y, boxA.size.y, boxB.offset.y, boxB.size.y)
		&& collide1d(boxA.offset.z, boxA.size.z, boxB.offset.z, boxB.size.z);
}

// for resolveCompare
double rankResolve(v3d res) {
	return fabs(res.x) + fabs(res.y) + fabs(res.z);
}

// for qsort
int resolveCompare(const void *a, const void *b) {
	v3d vA = *(v3d *)a, vB = *(v3d *)b;

	// avoid demotion rounding errors
	if (rankResolve(vA) - rankResolve(vB) < 0)
		return -1;
	return 1;
}

// assumes collision; pastes results into dest array
void findPossResolves(v3d dest[6], bbox_t eBox, bbox_t otherBox) {
	int i;
	double diff1, diff2;

	for (i = 0; i < 6; i++)
		dest[i] = (v3d){0, 0, 0};

	for (i = 0; i < 3; i++) {
		diff1 = (v3d_get(&otherBox.offset, i) + v3d_get(&otherBox.size, i)) - v3d_get(&eBox.offset, i);
		diff2 = v3d_get(&otherBox.offset, i) - (v3d_get(&eBox.offset, i) + v3d_get(&eBox.size, i));

		v3d_set(&(dest[i]), i, diff1);
		v3d_set(&(dest[i + 3]), i, diff2);
	}
}

// returns a resolution to kick eBox out of all the boxes in boxArr
// this is a clusterfuck. use the vector backtracking method.
v3d collideResolveMultiple(bbox_t eBox, bbox_t *boxArr, int lenArr) {
	int i, j, k;
	bool noCollide;
	bbox_t otherBox, resolveBox;
	v3d resolves[6], result = (v3d){0, 0, 0}, testResult;

	for (i = 0; i < lenArr; i++) {
		otherBox = boxArr[i];
		
		if (bboxCollide(eBox, otherBox)) {
			findPossResolves(resolves, eBox, otherBox);
			qsort(resolves, 6, sizeof(v3d), resolveCompare);

			printf("colliding with block: ");
			v3d_print(otherBox.offset);
			printf("\n");

			for (j = 0; j < 6; j++) {
				if (rankResolve(resolves[j]) > 1)
					continue;

				noCollide = true;
				resolveBox = eBox; // where eBox would be after resolution is applied
				//testResult = v3dAdd(resolves[j], result);
				resolveBox.offset = v3d_add(resolves[j], resolveBox.offset);
				
				/*
				for (k = 0; k < lenArr; k++) {
					if (k == i) {
						continue;
					} else if (bboxCollide(resolveBox, boxArr[k])) {
						noCollide = false;
						break;
					}
				}
				*/

				if (noCollide) {
					
					return resolves[j];
				}
			}

			//printf("got here\n");

			// collision is not yet resolvable
			//result = v3dAdd(resolves[0], result);
		} 
	}

	return (v3d){0, 0, 0}; 
}

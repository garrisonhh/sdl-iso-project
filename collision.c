#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "collision.h"
#include "vector.h"
#include "entity.h"
#include "world.h"

void printfBBox(bbox_t bbox) {
	printf("(bbox_t){");
	printfDvector3(bbox.offset);
	printf(", ");
	printfDvector3(bbox.size);
	printf("}");
}

bool collides(double a, double b, double x) {
	return (a < x) && (x < b);
}

bool collide1d(double startA, double lenA, double startB, double lenB) {
	return collides(startB, startB + lenB, startA)
		|| collides(startB, startB + lenB, startA + lenA)
		|| (startA == startB && lenA == lenB);
}

bool bboxCollide(bbox_t boxA, bbox_t boxB) {
	return collide1d(boxA.offset.x, boxA.size.x, boxB.offset.x, boxB.size.x)
		&& collide1d(boxA.offset.y, boxA.size.y, boxB.offset.y, boxB.size.y)
		&& collide1d(boxA.offset.z, boxA.size.z, boxB.offset.z, boxB.size.z);
}

// for resolveCompare
double rankResolve(dvector3 res) {
	return fabs(res.x) + fabs(res.y) + fabs(res.z);
}

// for qsort
int resolveCompare(const void *a, const void *b) {
	dvector3 vA = *(dvector3 *)a, vB = *(dvector3 *)b;

	// avoid demotion rounding errors
	if (rankResolve(vA) - rankResolve(vB) < 0)
		return -1;
	return 1;
}

// assumes collision; pastes results into dest
void findPossResolves(dvector3 dest[6], bbox_t eBox, bbox_t otherBox) {
	int i;
	double diff1, diff2;

	for (i = 0; i < 6; i++)
		dest[i] = (dvector3){0, 0, 0};

	for (i = 0; i < 3; i++) {
		diff1 = (dvector3Get(otherBox.offset, i) + dvector3Get(otherBox.size, i)) - dvector3Get(eBox.offset, i);
		diff2 = dvector3Get(otherBox.offset, i) - (dvector3Get(eBox.offset, i) + dvector3Get(eBox.size, i));

		dvector3Set(&(dest[i]), i, diff1);
		dvector3Set(&(dest[i + 3]), i, diff2);
	}
}

// returns a resolution to kick eBox out of all the boxes in boxArr
dvector3 collideResolveMultiple(bbox_t eBox, bbox_t *boxArr, int lenArr) {
	int i, j, k;
	bool noCollide;
	bbox_t otherBox, resolveBox;
	dvector3 resolves[6];

	for (i = 0; i < lenArr; i++) {
		otherBox = boxArr[i];
		
		if (bboxCollide(eBox, otherBox)) {
			findPossResolves(resolves, eBox, otherBox);
			qsort(resolves, 6, sizeof(dvector3), resolveCompare);

			for (j = 0; j < 6; j++) {
				noCollide = true;
				resolveBox = eBox; // where eBox would be after resolution is applied
				resolveBox.offset = dvector3Add(resolves[j], resolveBox.offset);
				
				for (k = 0; k < lenArr; k++) {
					if (k == i) {
						continue;
					} else if (bboxCollide(resolveBox, boxArr[k])) {
						noCollide = false;
						break;
					}
				}

				if (noCollide)
					return resolves[j];
			}
		} 
	}

	return (dvector3){0, 0, 0}; 
}

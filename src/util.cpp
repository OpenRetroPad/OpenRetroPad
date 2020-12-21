
uint8_t calculateDpadDirection(const bool up, const bool down, const bool left, const bool right) {
	if (down) {
		if (right) {
			return DPAD_DOWN_RIGHT;
		} else if (left) {
			return DPAD_DOWN_LEFT;
		} else {
			return DPAD_DOWN;
		}
	} else if (up) {
		if (right) {
			return DPAD_UP_RIGHT;
		} else if (left) {
			return DPAD_UP_LEFT;
		} else {
			return DPAD_UP;
		}
	} else if (right) {
		return DPAD_RIGHT;
	} else if (left) {
		return DPAD_LEFT;
	} else {
		return DPAD_CENTERED;
	}
}

struct Axis {
	int16_t x;
	int16_t y;
};

struct Axis axis(int16_t x, int16_t y) {
	Axis axis;
	axis.x = x;
	axis.y = y;
	return axis;
}

struct Axis dpadToAxis(uint8_t dpad) {
	switch (dpad) {
		case DPAD_CENTER:
			return axis(AXIS_CENTER, AXIS_CENTER);
		case DPAD_UP:
			return axis(AXIS_CENTER, AXIS_MIN);
		case DPAD_UP_RIGHT:
			return axis(AXIS_MAX, AXIS_MAX);
		case DPAD_RIGHT:
			return axis(AXIS_MAX, AXIS_CENTER);
		case DPAD_DOWN_RIGHT:
			return axis(AXIS_MAX, AXIS_MAX);
		case DPAD_DOWN:
			return axis(AXIS_CENTER, AXIS_MAX);
		case DPAD_DOWN_LEFT:
			return axis(AXIS_MIN, AXIS_MAX);
		case DPAD_LEFT:
			return axis(AXIS_MIN, AXIS_CENTER);
		case DPAD_UP_LEFT:
			return axis(AXIS_MIN, AXIS_MIN);
	}
	// todo: panic here?
	return axis(AXIS_CENTER, AXIS_CENTER);
}

/*
long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
*/

inline int16_t translateAxis(int16_t v) {
	// don't map at all if translation isn't required...
#if AXIS_CENTER_IN == AXIS_CENTER && AXIS_MIN_IN == AXIS_MIN && AXIS_MAX_IN == AXIS_MAX
	return v;  // noop
#else
	//return v == AXIS_CENTER_IN ? AXIS_CENTER : map(v, AXIS_MIN_IN, AXIS_MAX_IN, AXIS_MIN, AXIS_MAX);
	return v == AXIS_CENTER_IN ? AXIS_CENTER : (v - AXIS_MIN_IN) * (AXIS_MAX - AXIS_MIN) / (AXIS_MAX_IN - AXIS_MIN_IN) + AXIS_MIN;
#endif
}

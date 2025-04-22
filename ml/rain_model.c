int predict_rain(float features[3]) {
    if (features[1] <= 60.00) {
        return 0;
    } else {
        if (features[1] <= 60.09) {
            if (features[0] <= 47.00) {
                if (features[0] <= 43.85) {
                    return 0;
                } else {
                    return 1;
                }
            } else {
                if (features[0] <= 87.60) {
                    return 0;
                } else {
                    return 1;
                }
            }
        } else {
            if (features[0] <= 99.93) {
                if (features[1] <= 99.83) {
                    return 1;
                } else {
                    return 1;
                }
            } else {
                if (features[2] <= 982.39) {
                    return 1;
                } else {
                    return 0;
                }
            }
        }
    }
}
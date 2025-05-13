# 🌧️ USA Rainfall Prediction Dataset (2024–2025)

The dataset provides detailed weather data collected from 20 major cities in the United States during the years 2024 and 2025.

## Dataset Content

- Over 2 years of daily data
- Each row represents a specific day in one of the monitored cities
- Includes key attributes such as:
  - Date
  - Location
  - Temperature
  - Humidity
  - Wind speed
  - Cloud cover
  - Atmospheric pressure
  - Precipitation

## 🎯 Objective

The dataset's target is the **`RainTomorrow`** column, a binary label:
- `1`: It rained the following day
- `0`: It did not rain the following day

This makes it particularly suitable for binary classification machine learning problems.

## Additional Notes

- Check and possibly handle missing or anomalous values
- Class balancing
- Dates are in `YYYY-MM-DD` format
- The original CSV file is not included in the repository (added to `.gitignore`)

## Source

Dataset available on [Kaggle](https://www.kaggle.com/datasets/waqi786/usa-rainfall-prediction-dataset-2024-2025)


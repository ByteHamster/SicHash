library(ggplot2)
require(plyr)
library(dplyr)
require(likert)
library(ggplot2)
library(gridExtra)
library(lubridate)
library(ggpubr)
library(showtext)
library(stringr)

rawData <- read.csv("threshold-gradient-0.98.csv",header=T,sep=";",stringsAsFactors=FALSE)

plot <- ggplot(data=rawData, aes(x=threshold0, y=threshold1)) +
  geom_tile(aes(fill=cost), colour = "#cccccc", size=0.1) +
  labs(x="% of 1-bit", y="% of 2-bit", fill="successful builds") +
  theme_minimal() +
  scale_fill_viridis_c(option = "magma") +
  expand_limits(x = c(0, 100), y = c(0, 100)) +
  theme()
plot(plot)
print(min(rawData$cost))

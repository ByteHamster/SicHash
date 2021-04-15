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

rawData <- read.csv("threshold-success-cuckoo-0.95-N2000.csv",header=T,sep=";",comment.char='#',stringsAsFactors=FALSE)
rawData <- rawData[rawData$succ > 100,]

plot <- ggplot(data=rawData, aes(x=perc1, y=perc2)) +
  geom_tile(aes(fill=succ, colour = cost<=min(cost)+0.02), size=0.5) +
  #geom_tile(aes(fill=succ), colour = "#cccccc", size=0.1) +
  labs(x="% of 1-bit", y="% of 2-bit", fill="successful builds") +
  theme_minimal() +
  scale_fill_viridis_c(option = "magma") +
  expand_limits(x = c(0, 100), y = c(0, 100)) +
  theme()
plot(plot)
print(min(rawData$cost))

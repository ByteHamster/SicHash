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

rawData <- read.csv("var-success-K.csv",header=T,sep=";",stringsAsFactors=FALSE)

plot <- ggplot(data=rawData, aes(x=threshold, y=success, color=factor(k))) +
  geom_line() +
  labs(x="Space usage", y="Success %", color="K") +
  theme_minimal() +
  theme()
plot(plot)

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

rawData <- read.csv("threshold-cuckoo-multilayer-0.98-just3.csv",header=T,sep=";",comment.char='#',stringsAsFactors=FALSE)
#rawData[rawData$elementClass=="bump",]$classCost <- 4.8
#rawData[rawData$elementClass=="bumpR",]$classCost <- 4.8

filtered<-group_by(rawData, run)
cost<-summarise(filtered, cost=sum(percentage/100 * as.numeric(classCost)))
minCost<-min(cost$cost)
maxCost<-max(cost$cost)
cost$cost<-cost$cost - minCost
normalizer <- 100 / max(cost$cost)

plot <- ggplot() +
  geom_point(data=rawData, aes(x=elementClass, y=percentage), position=position_jitter(w=0.18, h=0)) +
  geom_point(data=cost, aes(x="Space", y=cost*normalizer), color="#0000dd", position=position_jitter(w=0.1, h=0)) +
  labs(x="Retrieval structure with memory usage", y="Percentage of elements in structure") +
  theme_minimal() +
  expand_limits(y = c(0, 100)) +
  scale_y_continuous(breaks=seq(0, 100, 10),
    sec.axis=sec_axis(trans=~./normalizer+minCost, name='Bits', breaks=seq(round(minCost, 2), maxCost, 0.02))) +
  theme(axis.text.y.right=element_text(color="#0000dd"), axis.title.y.right=element_text(color="#0000dd")) + 
  stat_summary(data=rawData, aes(x=elementClass, y=percentage),
     geom = "point", fun = "median", col = "red", size = 1.5, shape = 24, fill = "red") +
  stat_summary(data=cost, aes(x="Space", y=cost*normalizer),
     geom = "point", fun = "median", col = "red", size = 1.5, shape = 24, fill = "red")
plot(plot)

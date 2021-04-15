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

rawData <- read.csv("hashfunctions-mine.csv",header=T,sep=";",stringsAsFactors=FALSE)
rawData$X0 <- as.numeric(rawData$X0)
data <- as.data.frame(table(rawData$X0))

plot <- ggplot(data=data, aes(x=Freq, y=factor(2), fill=forcats::fct_rev(factor(factor(Var1))))) +
  geom_bar(stat="identity", colour="white",size=0.2) +
  scale_fill_discrete(limits=factor(0:14))+
  labs(x="Buckets using hash function", y="Fill percentage", fill="Hash function") +
  theme_minimal() +
  theme(legend.position='none')
plot(plot)

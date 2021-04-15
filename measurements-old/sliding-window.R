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

rawData <- read.csv("sliding-window.csv",header=T,sep=";",stringsAsFactors=FALSE)
data <- as.data.frame(table(rawData$Hash, rawData$ID))

plot <- ggplot(data=data, aes(x=Freq, y=factor(Var2), fill=forcats::fct_rev(factor(factor(Var1))))) +
  geom_bar(stat="identity", colour="white",size=0.2) +
  scale_fill_discrete(limits=factor(0:14))+
  labs(x="Buckets using hash function", y="Fill percentage", fill="Hash function") +
  theme_minimal() +
  theme(legend.position='none')
plot(plot)

bitsHashPerBucket = 11
for (ID in unique(data$Var2)) {
  filtered <- data[data$Var2==ID,]
  bumped = sum(filtered[filtered$Var1 == -1,]$Freq) +
    sum(filtered[as.numeric(filtered$Var1) > (2 ** bitsHashPerBucket),]$Freq)
  elements = sum(rawData[rawData$ID==ID,]$Elements)
  numBuckets = max(rawData$Bucket) + 1
  bucketSize = elements/numBuckets
  space = numBuckets*bitsHashPerBucket + bumped*bucketSize*32 # Store explicitly
  bits = space/elements
  print(paste(ID, bits, sep=": "))
  nonZeroValues <- filtered[filtered$Freq!=0,]$Freq
  print(-sum(nonZeroValues/numBuckets * log2(nonZeroValues/numBuckets)))
}

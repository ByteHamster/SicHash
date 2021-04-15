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

data <- read.csv("matching.csv",header=T,sep=";",stringsAsFactors=FALSE)

plot <- ggplot(data=data, aes(x=N, y=ID, fill=forcats::fct_rev(factor(i)))) +
  geom_bar(stat="identity", colour="white",size=0.2) +
  scale_fill_discrete(limits=0:15)+
  labs(x="Items using hash function", y="Method", fill="Hash function") +
  theme_minimal() +
  theme(legend.position='none')
plot(plot)

for (ID in unique(data$ID)) {
  items <- data[data$ID == ID,]
  items$Probabil <- items$N / sum(items$N)
  entropy <- -sum(items$Probabil * log2(items$Probabil))
  print(paste(ID, entropy, sep=": "))
}

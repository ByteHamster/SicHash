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

rawData <- read.csv("scatterplot.csv",header=T,sep=";",comment.char='#',stringsAsFactors=FALSE)
rawData[rawData$size>2.1,]$size <- 2.1

LABELS <- function(x){
  ifelse(x==2.1, "More", x)
}

plot <- ggplot() +
  geom_point(data=rawData, aes(x=size, y=time, color=Method, shape=Method)) +
  labs(x="Bits/element", y="Construction time (seconds)",
       color="Method", shape="Method") +
  theme_minimal() +
  geom_vline(xintercept=2.08, linetype="dashed") +
  scale_shape_manual(values=seq(0,15)) +
  scale_x_continuous(label=LABELS)
#plot(plot)
ggsave("scatterplot.pdf",plot, width=6, height=3, units="cm", scale=3)

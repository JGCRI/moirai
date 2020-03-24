library(dplyr)
library(data.table)
library(testthat)

#Add links to datasets
#1. The original dataset
Original_LDS_Data<-read.csv("D:/MOIRAI/moirai/outputs/basins235/Land_type_area_ha.csv",skip = 5)
#2. Updated dataset
Updated_LDS_Data<-read.csv("D:/MOIRAI_3.1_Final/moirai/ancillary/Land_type_area_ha _Updated_withIUCN_Categories.csv",skip = 5)
#3. Old land types
MOIRAI_Types_old<-read.csv("D:/MOIRAI/moirai/outputs/basins235/MOIRAI_land_types.csv",skip=4)
#4. New land types
MOIRAI_Types_new<-read.csv("D:/MOIRAI_3.1_Final/moirai/outputs/basins235_lulcc/MOIRAI_land_types.csv",skip = 4)


#Add fast group_by function
fast_group_by<- function(df,by,colname="value",func= "sum"){


  #Convert relevant column to numeric
  df[,colname]<- as.numeric(df[,colname])

  #Store as data.table
  df <- as.data.table(df)

  #Complete operations
  df<- df[, (colname) := (get(func)(get(colname))), by]

  #Save back to tibble
  df<- as_tibble(df)

  return(df)
}


#First compare by Country_Year
Original_LDS_Data %>%
  fast_group_by(by=c("iso","year"),colname = "value",func="sum") %>%
  select(iso,year,value) %>%
  distinct()->ISO_Year_Data_Old

Updated_LDS_Data %>%
  fast_group_by(by=c("iso","year"),colname = "value",func="sum") %>%
  select(iso,year,value) %>%
  rename(Updated_Value=value) %>%
  distinct()->ISO_Year_Data_New

test_that("Compare difference by year and value",{

  expect_equal(ISO_Year_Data_Old$value,ISO_Year_Data_New$Updated_Value,tolerance=0.01,info=paste("ISO year differences are too high."))

})


#Compare by iso,glu,year
Original_LDS_Data %>%
  fast_group_by(by=c("iso","glu_code","year"),colname = "value",func="sum") %>%
  select(iso,glu_code,year,value) %>%
  distinct()->ISO_GLU_Year_Data_Old

Updated_LDS_Data %>%
  fast_group_by(by=c("iso","glu_code","year"),colname = "value",func="sum") %>%
  select(iso,glu_code,year,value) %>%
  rename(Updated_Value=value) %>%
  distinct()->ISO_GLU_Year_Data_New

test_that("Compare difference by year and value",{

  expect_equal(ISO_GLU_Year_Data_Old$value,ISO_GLU_Year_Data_New$Updated_Value,tolerance=0.01,info=paste("ISO year differences are too high."))

})

#Compare Hyde type
Original_LDS_Data %>%
  left_join(MOIRAI_Types_old %>% select(Category,LT_HYDE) %>% rename(land_type=Category),by=c("land_type")) %>%
  fast_group_by(by=c("iso","glu_code","LT_HYDE","year"),colname = "value",func="sum") %>%
  select(iso,glu_code,year,LT_HYDE,value) %>%
  distinct() %>%
  filter(value>2)->ISO_GLU_LT_Year_Data_Old

Updated_LDS_Data %>%
  left_join(MOIRAI_Types_new %>% select(Category,LT_HYDE) %>% rename(land_type=Category),by=c("land_type")) %>%
  fast_group_by(by=c("iso","glu_code","LT_HYDE","year"),colname = "value",func="sum") %>%
  select(iso,glu_code,year,LT_HYDE,value) %>%
  rename(Updated_Value=value) %>%
  distinct() %>%
  filter(Updated_Value>2)->ISO_GLU_LT_Year_Data_New

ISO_GLU_LT_Year_Data_Old %>% left_join(ISO_GLU_LT_Year_Data_New,by=c("iso","glu_code","LT_HYDE","year")) %>%
  mutate(Difference=Updated_Value-value) %>%
  mutate(Difference_Percent=(Difference/value)*100) %>%
  filter(value>50)->tmp

write.csv(tmp,"Hyde_Differences.csv")


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
#5. Old carbon file
Old_Carbon_Data<- read.csv("D:/MOIRAI/moirai/outputs/basins235/Ref_veg_carbon_Mg_per_ha.csv",skip = 5)
#6. New carbon file
New_Carbon_Data<- read.csv("D:/MOIRAI_3.1_Final/moirai/outputs/basins235_lulcc/Ref_veg_carbon_Mg_per_ha.csv",skip = 5)



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
  left_join(MOIRAI_Types_old %>% select(Category,LT_HYDE,LT_SAGE) %>% rename(land_type=Category),by=c("land_type")) %>%
  fast_group_by(by=c("iso","glu_code","LT_HYDE","year","LT_SAGE"),colname = "value",func="sum") %>%
  select(iso,glu_code,year,LT_HYDE,value,LT_SAGE) %>%
  distinct() %>%
  filter(value>2)->ISO_GLU_LT_Year_Data_Old

Updated_LDS_Data %>%
  left_join(MOIRAI_Types_new %>% select(Category,LT_HYDE,LT_SAGE) %>% rename(land_type=Category),by=c("land_type")) %>%
  fast_group_by(by=c("iso","glu_code","LT_HYDE","year","LT_SAGE"),colname = "value",func="sum") %>%
  select(iso,glu_code,year,LT_HYDE,value,LT_SAGE) %>%
  rename(Updated_Value=value) %>%
  distinct() %>%
  filter(Updated_Value>2)->ISO_GLU_LT_Year_Data_New

ISO_GLU_LT_Year_Data_Old %>% left_join(ISO_GLU_LT_Year_Data_New,by=c("iso","glu_code","LT_HYDE","year","LT_SAGE")) %>%
  mutate(Difference=abs(Updated_Value-value)) %>%
  mutate(Difference_Percent=(Difference/value)*100) %>%
  filter(value>5)->tmp

g<-ggplot(data=tmp,aes(x=value,y=Updated_Value))+
   geom_point()+
   ggtitle("Comparing LDS outputs between old and new across all years")+
   labs(subtitle = "This is a comparison at the lowest level i.e Year+ISO+GLU+SAGE_type+Hyde_type")

g

tmp %>% filter(Difference>5) %>% filter(Difference_Percent>15)->Data_for_comparison


test_that("Compare difference at lowest level",{
  
  expect_equal(sum(nrow(Data_for_comparison)),0,info=paste("Differences at the lowest level are not reasonable"))
  
})

tmp %>% filter(year==1990) %>% filter(iso=="idn")->Indonesia_anomaly
write.csv(Indonesia_anomaly,"Indonesia_anomaly.csv")


#Part 2: Carbon accounting for soil_carbon, vegetation carbon
Old_Carbon_Data %>% 
  inner_join(Original_LDS_Data %>% 
               filter(year==2015) %>%
               select(iso,glu_code,land_type,value) %>% rename(Land_value=value), by=c("iso","glu_code","land_type")) %>% 
  mutate(Old_Carbon_Total = Land_value * value ) %>% 
  group_by(iso,glu_code,c_type) %>% 
  mutate(Old_Carbon_Total=sum(Old_Carbon_Total)) %>% 
  select(iso,glu_code,c_type,Old_Carbon_Total) %>% 
  distinct()->Old_Total_Carbon
                                                                  
New_Carbon_Data %>% 
  inner_join(Updated_LDS_Data %>% 
               filter(year==2015) %>%
               select(iso,glu_code,land_type,value) %>% rename(Land_value=value), by=c("iso","glu_code","land_type")) %>% 
  mutate(New_Carbon_Total = Land_value * value ) %>% 
  group_by(iso,glu_code,c_type) %>% 
  mutate(New_Carbon_Total=sum(New_Carbon_Total)) %>% 
  select(iso,glu_code,c_type,New_Carbon_Total) %>% 
  distinct()->New_Total_Carbon                                   

New_Total_Carbon %>% 
  inner_join(Old_Total_Carbon,by=c("iso","glu_code","c_type")) %>%
  mutate(New_Carbon_Total=ifelse(is.na(New_Carbon_Total),0,New_Carbon_Total)) %>% 
  mutate(Difference=abs(New_Carbon_Total-Old_Carbon_Total)) %>% 
  mutate(Percent_Difference=(Difference/Old_Carbon_Total)*100)->Carbon_Comparison

g<-ggplot(data=Carbon_Comparison,aes(x=Old_Carbon_Total,y=New_Carbon_Total))+
  geom_point()+
  ggtitle("Comparing carbon outputs between old and new across all years")+
  labs(subtitle = "This is a comparison at the lowest level i.e Year+ISO+GLU+SAGE_type+Hyde_type")+
  facet_wrap(~c_type)

g

Carbon_Comparison %>% filter(Difference>4000000) %>% 
                      filter(Percent_Difference>15)->Carbon
Carbon_Comparison %>% filter(iso=="ecu") %>%filter(c_type=="soil_c")->Ecuador_example 

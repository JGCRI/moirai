library(dplyr)
library(data.table)
library(testthat)
library(ggplot2)

###### update this when done to not reflect my directory structure!!!!
setwd("~/projects/gcam_lds/moirai/diagnostics")

#Add links to datasets
#1. The original dataset
Original_LDS_Data<-read.csv("../outputs/basins235_lulcc/Land_type_area_ha.csv",skip = 5, stringsAsFactors=FALSE)
#2. Updated dataset
Updated_LDS_Data<-read.csv("../outputs/basins235_test_new_protected/Land_type_area_ha.csv",skip = 5, stringsAsFactors=FALSE)
#3. Old land types
MOIRAI_Types_old<-read.csv("../outputs/basins235_lulcc/MOIRAI_land_types.csv",skip=4, stringsAsFactors=FALSE)
#4. New land types
MOIRAI_Types_new<-read.csv("../outputs/basins235_test_new_protected/MOIRAI_land_types.csv",skip = 4, stringsAsFactors=FALSE)
#5. Old carbon file
Old_Carbon_Data<- read.csv("../outputs/basins235_lulcc/Ref_veg_carbon_Mg_per_ha.csv",skip = 5, stringsAsFactors=FALSE)
#6. New carbon file
New_Carbon_Data<- read.csv("../outputs/basins235_test_new_protected/Ref_veg_carbon_Mg_per_ha.csv",skip = 5, stringsAsFactors=FALSE)




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
  dplyr::select(iso,year,value) %>%
  distinct()->ISO_Year_Data_Old

Updated_LDS_Data %>%
  fast_group_by(by=c("iso","year"),colname = "value",func="sum") %>%
  dplyr::select(iso,year,value) %>%
  rename(Updated_Value=value) %>%
  distinct()->ISO_Year_Data_New

# expect_equal includes only records with differences, and checks for mean(abs(x-y))/mean(abs(y)) < tolerance, where x is the vector of data to check and y is the vector of expected values
# the data pass the 1e-5 tolerance, but not 1e-6. the machine default tolerance on a mac desktop is sqrt(.Machine$double.eps)=1.490116e-8
# while this is a useful test, we need to check that each absolute and relative differnce are less than reasonable thresholds
test_that("Compare difference by year and value",{

  expect_equal(ISO_Year_Data_New$Updated_Value,ISO_Year_Data_Old$value,tolerance=1e-5,info=paste("ISO year differences are too high."))

})


#Compare by iso,glu,year
Original_LDS_Data %>%
  fast_group_by(by=c("iso","glu_code","year"),colname = "value",func="sum") %>%
  dplyr::select(iso,glu_code,year,value) %>%
  distinct()->ISO_GLU_Year_Data_Old

Updated_LDS_Data %>%
  fast_group_by(by=c("iso","glu_code","year"),colname = "value",func="sum") %>%
  dplyr::select(iso,glu_code,year,value) %>%
  rename(Updated_Value=value) %>%
  distinct()->ISO_GLU_Year_Data_New

test_that("Compare difference by year and value",{

  expect_equal(ISO_GLU_Year_Data_New$Updated_Value,ISO_GLU_Year_Data_Old$value,tolerance=1e-5,info=paste("ISO year differences are too high."))

})


#Compare Hyde type - to get to gcam nat veg cats need to aggregate the Unmanaged separately
Original_LDS_Data %>%
  left_join(MOIRAI_Types_old %>% dplyr::select(Category,LT_HYDE,LT_SAGE) %>% rename(land_type=Category),by=c("land_type")) %>%
  fast_group_by(by=c("iso","glu_code","LT_HYDE","year"),colname = "value",func="sum") %>%
  dplyr::select(iso,glu_code,year,LT_HYDE,value) %>%
  distinct() %>%
  filter(value>0)->ISO_GLU_HYDE_Year_Data_Old

Updated_LDS_Data %>%
  left_join(MOIRAI_Types_new %>% dplyr::select(Category,LT_HYDE,LT_SAGE) %>% rename(land_type=Category),by=c("land_type")) %>%
  fast_group_by(by=c("iso","glu_code","LT_HYDE","year"),colname = "value",func="sum") %>%
  dplyr::select(iso,glu_code,year,LT_HYDE,value) %>%
  rename(Updated_Value=value) %>%
  distinct() %>%
  filter(Updated_Value>0)->ISO_GLU_HYDE_Year_Data_New

ISO_GLU_HYDE_Year_Data_Old %>% full_join(ISO_GLU_HYDE_Year_Data_New,by=c("iso","glu_code","LT_HYDE","year")) -> tmp1
z=which(is.na(tmp1$value))
if(length(z)>0) {tmp1$value[z] = 0}
z=which(is.na(tmp1$Updated_Value))
if(length(z)>0) {tmp1$Updated_Value[z] = 0}
mutate(tmp1, Difference=Updated_Value-value) %>%
  mutate(Difference_Percent=(Difference/value)*100) -> tmp1
  
test_that("Compare difference at lowest level",{
  
  expect_equal(tmp1$Updated_Value,tmp1$value, tolerance=1e-5,info=paste("Differences at the lowest level are not reasonable"))
  
})

tmp1 %>% filter(abs(Difference)>3)->high_value_diff1
max(abs(high_value_diff1$Difference_Percent))

tmp1 %>% filter(abs(Difference_Percent)>0.5)->high_percent_diff1
max(abs(high_percent_diff1$Difference))

tmp1 %>% filter(abs(Difference_Percent)>1) %>% filter(abs(Difference)>1) -> Data_for_comparison1
max(abs(Data_for_comparison1$Difference))

g<-ggplot(data=tmp1,aes(x=value,y=Updated_Value))+
   geom_point()+
   ggtitle("Comparing LDS outputs between old and new across all years")+
   labs(subtitle = "This is a comparison at the hyde level i.e Year+ISO+GLU+Hyde_type")

g




#Compare SAGE type
Original_LDS_Data %>%
  left_join(MOIRAI_Types_old %>% dplyr::select(Category,LT_HYDE,LT_SAGE) %>% rename(land_type=Category),by=c("land_type")) %>%
  fast_group_by(by=c("iso","glu_code","LT_SAGE","year"),colname = "value",func="sum") %>%
  dplyr::select(iso,glu_code,year,LT_SAGE,value) %>%
  distinct() %>%
  filter(value>0)->ISO_GLU_SAGE_Year_Data_Old

Updated_LDS_Data %>%
  left_join(MOIRAI_Types_new %>% dplyr::select(Category,LT_HYDE,LT_SAGE) %>% rename(land_type=Category),by=c("land_type")) %>%
  fast_group_by(by=c("iso","glu_code","LT_SAGE","year"),colname = "value",func="sum") %>%
  dplyr::select(iso,glu_code,year,LT_SAGE,value) %>%
  rename(Updated_Value=value) %>%
  distinct() %>%
  filter(Updated_Value>0)->ISO_GLU_SAGE_Year_Data_New

ISO_GLU_SAGE_Year_Data_Old %>% full_join(ISO_GLU_SAGE_Year_Data_New,by=c("iso","glu_code","LT_SAGE","year")) -> tmp2
z=which(is.na(tmp2$value))
if(length(z)>0) {tmp2$value[z] = 0}
z=which(is.na(tmp2$Updated_Value))
if(length(z)>0) {tmp2$Updated_Value[z] = 0}
 mutate(tmp2, Difference=Updated_Value-value) %>%
  mutate(Difference_Percent=(Difference/value)*100) -> tmp2

test_that("Compare difference at lowest level",{
  
  expect_equal(tmp2$Updated_Value,tmp2$value, tolerance=1e-2,info=paste("Differences at the lowest level are not reasonable"))
  
})

tmp2 %>% filter(abs(Difference)>3)->high_value_diff2
max(abs(high_value_diff1$Difference_Percent))

tmp2 %>% filter(abs(Difference_Percent)>0.5)->high_percent_diff2
max(abs(high_percent_diff2$Difference))

tmp2 %>% filter(abs(Difference_Percent)>1) %>% filter(abs(Difference)>1) -> Data_for_comparison2
max(abs(Data_for_comparison2$Difference))

g<-ggplot(data=tmp2,aes(x=value,y=Updated_Value))+
   geom_point()+
   ggtitle("Comparing LDS outputs between old and new across all years")+
   labs(subtitle = "This is a comparison at the sage level i.e Year+ISO+GLU+SAGE")

g



#Compare Hyde and SAGE type
Original_LDS_Data %>%
  left_join(MOIRAI_Types_old %>% dplyr::select(Category,LT_HYDE,LT_SAGE) %>% rename(land_type=Category),by=c("land_type")) %>%
  fast_group_by(by=c("iso","glu_code","LT_HYDE","year","LT_SAGE"),colname = "value",func="sum") %>%
  dplyr::select(iso,glu_code,year,LT_HYDE,value,LT_SAGE) %>%
  distinct() %>%
  filter(value>0)->ISO_GLU_LT_Year_Data_Old

Updated_LDS_Data %>%
  left_join(MOIRAI_Types_new %>% dplyr::select(Category,LT_HYDE,LT_SAGE) %>% rename(land_type=Category),by=c("land_type")) %>%
  fast_group_by(by=c("iso","glu_code","LT_HYDE","year","LT_SAGE"),colname = "value",func="sum") %>%
  dplyr::select(iso,glu_code,year,LT_HYDE,value,LT_SAGE) %>%
  rename(Updated_Value=value) %>%
  distinct() %>%
  filter(Updated_Value>0)->ISO_GLU_LT_Year_Data_New

ISO_GLU_LT_Year_Data_Old %>% full_join(ISO_GLU_LT_Year_Data_New,by=c("iso","glu_code","LT_HYDE","year","LT_SAGE")) -> tmp
z=which(is.na(tmp$value))
if(length(z)>0) {tmp$value[z] = 0}
z=which(is.na(tmp$Updated_Value))
if(length(z)>0) {tmp$Updated_Value[z] = 0}
mutate(tmp, Difference=Updated_Value-value) %>%
  mutate(Difference_Percent=(Difference/value)*100) -> tmp

g<-ggplot(data=tmp,aes(x=value,y=Updated_Value))+
   geom_point()+
   ggtitle("Comparing LDS outputs between old and new across all years")+
   labs(subtitle = "This is a comparison at the lowest level i.e Year+ISO+GLU+SAGE_type+Hyde_type")

g

test_that("Compare difference at lowest level",{
  
  expect_equal(tmp$Updated_Value,tmp$value, tolerance=1e-2,info=paste("Differences at the lowest level are not reasonable"))
  
})

tmp %>% filter(abs(Difference)>3)->high_value_diff
max(abs(high_value_diff$Difference_Percent))
max(abs(high_value_diff$Difference_Percent[high_value_diff$Difference_Percent != Inf]))

tmp %>% filter(abs(Difference_Percent)>0.5)->high_percent_diff
max(abs(high_percent_diff$Difference))

tmp %>% filter(abs(Difference_Percent)>1) %>% filter(abs(Difference)>1) -> Data_for_comparison
max(abs(Data_for_comparison$Difference))



# Part 2: Carbon accounting for soil_carbon, vegetation carbon
# the carbon outputs are based on year 2000 reference vegetation area
# here compare only the commond land areas

# land unit level (countryXglu) in units of Mg C

Old_Carbon_Data %>% 
  inner_join(Original_LDS_Data %>% 
               filter(year==2000) %>%
               dplyr::select(iso,glu_code,land_type,value) %>% rename(Land_value=value), by=c("iso","glu_code","land_type")) %>% 
  mutate(Old_Carbon_Total = as.numeric(Land_value) * as.numeric(value) ) %>% 
  group_by(iso,glu_code,c_type) %>% 
  mutate(Old_Carbon_Total=sum(Old_Carbon_Total)) %>% 
  dplyr::select(iso,glu_code,c_type,Old_Carbon_Total) %>% 
  distinct()->Old_Total_Carbon
                                                                  
New_Carbon_Data %>% 
  inner_join(Updated_LDS_Data %>% 
               filter(year==2000) %>%
               dplyr::select(iso,glu_code,land_type,value) %>% rename(Land_value=value), by=c("iso","glu_code","land_type")) %>% 
  mutate(New_Carbon_Total = as.numeric(Land_value) * as.numeric(value) ) %>% 
  group_by(iso,glu_code,c_type) %>% 
  mutate(New_Carbon_Total=sum(New_Carbon_Total)) %>% 
  dplyr::select(iso,glu_code,c_type,New_Carbon_Total) %>% 
  distinct()->New_Total_Carbon                                   

New_Total_Carbon %>% 
  inner_join(Old_Total_Carbon,by=c("iso","glu_code","c_type")) %>%
  mutate(New_Carbon_Total=ifelse(is.na(New_Carbon_Total),0,New_Carbon_Total)) %>% 
  mutate(Difference=New_Carbon_Total-Old_Carbon_Total) %>% 
  mutate(Percent_Difference=(Difference/Old_Carbon_Total)*100)->Carbon_Comparison

g<-ggplot(data=Carbon_Comparison,aes(x=Old_Carbon_Total,y=New_Carbon_Total))+
  geom_point()+
  ggtitle("Comparing carbon outputs between old and new for 2000")+
  labs(subtitle = "This is a comparison at the land unit level in 2000 i.e countryXglu")+
  facet_wrap(~c_type)

g


g<-ggplot(data=Carbon_Comparison[Carbon_Comparison$Old_Carbon_Total < 2e9,],aes(x=Old_Carbon_Total,y=New_Carbon_Total))+
  geom_point()+
  ggtitle("Comparing carbon outputs between old and new for 2000")+
  labs(subtitle = "This is a comparison at the land unit level in 2000 i.e countyXglu")+
  facet_wrap(~c_type)

g

test_that("Compare difference at land unit (countryXglu) level",{
  
  expect_equal(Carbon_Comparison$Old_Carbon_Total,Carbon_Comparison$New_Carbon_Total, tolerance=1e0,info=paste("Differences at the lowest level are not reasonable"))
  
})

Carbon_Comparison %>% filter(abs(Percent_Difference)>15) %>% filter(abs(Difference)>0)->Carbon_Land_Unit


# by iso and glu and HYDE and SAGE cats in units of MgC/ha

Old_Carbon_Data %>% 
  full_join(Original_LDS_Data %>% 
               filter(year==2000) %>%
               dplyr::select(iso,glu_code,land_type,value) %>% rename(Land_value=value), by=c("iso","glu_code","land_type")) %>% 
  left_join(MOIRAI_Types_old %>% dplyr::select(Category,LT_HYDE,LT_SAGE) %>% rename(land_type=Category),by=c("land_type")) %>%
  mutate(Old_Carbon_Total = as.numeric(Land_value) * as.numeric(value) ) -> t1
 # duplicate the NA rows and assign them c_type
 # this will also drop any land types that have only one of the carbon values defined
 ninds=which(is.na(t1$value))
 t1add = t1[ninds,]
 t1add$c_type = "veg_c"
 t1$c_type[ninds] = "soil_c"
 t1=rbind(t1,t1add)
 Old_Carbon = aggregate(. ~ iso + glu_code + c_type + LT_HYDE + LT_SAGE, t1[,c("iso", "glu_code", "c_type", "LT_HYDE", "LT_SAGE", "Land_value", "Old_Carbon_Total")], FUN=sum, na.action=na.pass)
 Old_Carbon = Old_Carbon[!is.na(Old_Carbon$Old_Carbon_Total),] 
 Old_Carbon$Old_Carbon_avg = Old_Carbon$Old_Carbon_Total / Old_Carbon$Land_value

                                                                  
New_Carbon_Data %>% 
  full_join(Updated_LDS_Data %>% 
               filter(year==2000) %>%
               dplyr::select(iso,glu_code,land_type,value) %>% rename(New_Land_value=value), by=c("iso","glu_code","land_type")) %>%
  left_join(MOIRAI_Types_new %>% dplyr::select(Category,LT_HYDE,LT_SAGE) %>% rename(land_type=Category),by=c("land_type")) %>%
  mutate(New_Carbon_Total = as.numeric(New_Land_value) * as.numeric(value) ) -> t 
  # duplicate the NA rows and assign them c_type
 # this will also drop any land types that have only one of the carbon values defined
 # this probably isn't needed any more
 ninds=which(is.na(t$value))
 tadd = t[ninds,]
 tadd$c_type = "veg_c"
 t$c_type[ninds] = "soil_c"
 t=rbind(t,tadd)
  New_Carbon = aggregate(. ~ iso + glu_code + c_type + LT_HYDE + LT_SAGE, t[,c("iso", "glu_code", "c_type", "LT_HYDE", "LT_SAGE", "New_Land_value", "New_Carbon_Total")], FUN=sum, na.action=na.pass)
  New_Carbon = New_Carbon[!is.na(New_Carbon$New_Carbon_Total),]
  New_Carbon$New_Carbon_avg = New_Carbon$New_Carbon_Total / New_Carbon$New_Land_value
                                 

New_Carbon %>% 
  inner_join(Old_Carbon,by=c("iso","glu_code","LT_HYDE","LT_SAGE","c_type")) %>%
  mutate(New_Carbon_Total=ifelse(is.na(New_Carbon_Total),0,New_Carbon_Total)) %>%
  mutate(Difference=New_Carbon_avg-Old_Carbon_avg) %>% 
  mutate(Percent_Difference=(Difference/Old_Carbon_avg)*100)->Carbon_Comparison


g<-ggplot(data=Carbon_Comparison[Carbon_Comparison$Old_Carbon_Total > 0,],aes(x=Old_Carbon_avg,y=New_Carbon_avg))+
  geom_point()+
  ggtitle("Comparing carbon outputs between old and new for 2000")+
  labs(subtitle = "This is a comparison at the lowest level i.e Year+ISO+GLU+SAGE_type+Hyde_type")+
  facet_wrap(~c_type)

g

Carbon_Comparison %>% filter(abs(Percent_Difference)>15) %>% filter(abs(Difference)>0)->Carbon

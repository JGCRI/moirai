# Created by Kanishka Narayan and Alan Di Vittorio
 
# Moirai Land Data System (Moirai) Copyright (c) 2020, The
# Regents of the University of California, through Lawrence Berkeley National
# Laboratory (subject to receipt of any required approvals from the U.S.
# Dept. of Energy).  All rights reserved.
 
# If you have questions about your rights to use or distribute this software,
# please contact Berkeley Lab's Intellectual Property Office at
# IPO@lbl.gov.
 
# NOTICE.  This Software was developed under funding from the U.S. Department
# of Energy and the U.S. Government consequently retains certain rights.  As
# such, the U.S. Government has been granted for itself and others acting on
# its behalf a paid-up, nonexclusive, irrevocable, worldwide license in the
# Software to reproduce, distribute copies to the public, prepare derivative
# works, and perform publicly and display publicly, and to permit other to do
# so.
 
# This file is part of Moirai.
 
# Moirai is free software: you can use it under the terms of the modified BSD-3 license (see …/moirai/license.txt)

library(dplyr)
library(data.table)
library(testthat)
library(ggplot2)

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




compare_iso_land_data <- function(path_to_Original_LDS_Data = "../example_outputs/basins235/Land_type_area_ha.csv",
                                  path_to_Updated_LDS_Data = "../example_outputs/basins235/Land_type_area_ha.csv",
                                  path_to_old_mapping = "../example_outputs/basins235/MOIRAI_land_types.csv",
                                  path_to_new_mapping = "../example_outputs/basins235/MOIRAI_land_types.csv",
                                  error_tolerance= 0.01,
                                  create_land_plot=TRUE){


#Add links to datasets
#1. The original dataset
Original_LDS_Data<-read.csv(path_to_Original_LDS_Data,skip = 5, stringsAsFactors=FALSE)
#2. Updated dataset
Updated_LDS_Data<-read.csv(path_to_Updated_LDS_Data,skip = 5, stringsAsFactors=FALSE)
#3. Old land types
MOIRAI_Types_old<-read.csv(path_to_old_mapping,skip=4, stringsAsFactors=FALSE)
#4. New land types
MOIRAI_Types_new<-read.csv(path_to_new_mapping,skip = 4, stringsAsFactors=FALSE)

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

if(create_land_plot==TRUE){
  
  ISO_Year_Data_Old %>% 
    full_join(ISO_Year_Data_New,by=c("iso","year")) %>% 
    mutate(Updated_Value=if_else(is.na(Updated_Value),0,Updated_Value)) %>% 
    mutate(value=if_else(is.na(value),0,value))->plot_data

  
  g<-ggplot(data=plot_data,aes(x=value,y=Updated_Value))+
    geom_point()+
    ggtitle("Comparing LDS outputs between old and new across all years")+
    labs(subtitle = "This is a comparison at the hyde level i.e Year+ISO")
  
  plot(g)  
  
  
  
}

# expect_equal includes only records with differences, and checks for mean(abs(x-y))/mean(abs(y)) < tolerance, where x is the vector of data to check and y is the vector of expected values
# the data pass the 1e-5 tolerance, but not 1e-6. the machine default tolerance on a mac desktop is sqrt(.Machine$double.eps)=1.490116e-8
# while this is a useful test, we need to check that each absolute and relative differnce are less than reasonable thresholds
return(expect_equal(ISO_Year_Data_New$Updated_Value,ISO_Year_Data_Old$value,tolerance=error_tolerance,info=paste("ISO year differences are too high.")))
}


compare_iso_glu_land_data<-function(path_to_Original_LDS_Data = "../example_outputs/basins235/Land_type_area_ha.csv",
                     path_to_Updated_LDS_Data = "../example_outputs/basins235/Land_type_area_ha.csv",
                     path_to_old_mapping = "../example_outputs/basins235/MOIRAI_land_types.csv",
                     path_to_new_mapping = "../example_outputs/basins235/MOIRAI_land_types.csv",
                     error_tolerance= 0.01,
                     create_land_plot=TRUE){

  #Add links to datasets
  #1. The original dataset
  Original_LDS_Data<-read.csv(path_to_Original_LDS_Data,skip = 5, stringsAsFactors=FALSE)
  #2. Updated dataset
  Updated_LDS_Data<-read.csv(path_to_Updated_LDS_Data,skip = 5, stringsAsFactors=FALSE)
  #3. Old land types
  MOIRAI_Types_old<-read.csv(path_to_old_mapping,skip=4, stringsAsFactors=FALSE)
  #4. New land types
  MOIRAI_Types_new<-read.csv(path_to_new_mapping,skip = 4, stringsAsFactors=FALSE)
  

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

if (nrow(Original_LDS_Data)>nrow(Updated_LDS_Data)){
  
  print("New data is missing values for following ISO GLU Year combinations")
  
  tmp<- ISO_GLU_Year_Data_Old %>% 
        left_join(ISO_GLU_Year_Data_New, by=c("iso","glu_code","year"))
  
  tmpna <- tmp[is.na(tmp$Updated_Value),]
  print(tmpna)  
}


if (nrow(Original_LDS_Data)<nrow(Updated_LDS_Data)){
  
  print("New data contains new values for following ISO GLU Year combinations")
  
  tmp<- ISO_GLU_Year_Data_New %>% 
    left_join(ISO_GLU_Year_Data_Old, by=c("iso","glu_code","year"))
  
  tmpna <- tmp[is.na(tmp$value),]
  print(tmpna)  
}


if (create_land_plot==TRUE){
  
  ISO_GLU_Year_Data_Old %>% 
    full_join(ISO_GLU_Year_Data_New,by=c("iso","glu_code","year")) %>% 
    mutate(Updated_Value= if_else(is.na(Updated_Value),0,Updated_Value)) %>%
    mutate(value= if_else(is.na(value),0,value)) ->plot_data
  
  g<-ggplot(data=plot_data,aes(x=value,y=Updated_Value))+
    geom_point()+
    ggtitle("Comparing LDS outputs between old and new across all years")+
    labs(subtitle = "This is a comparison at the hyde level i.e Year+ISO_GLU level")
  
  plot(g)  
  
  
}

return(expect_equal(ISO_GLU_Year_Data_New$Updated_Value,ISO_GLU_Year_Data_Old$value,tolerance=error_tolerance,info=paste("ISO year differences are too high.")))

}


compare_iso_glu_hyde_land_data<-function(path_to_Original_LDS_Data = "../example_outputs/basins235/Land_type_area_ha.csv",
                                         path_to_Updated_LDS_Data = "../example_outputs/basins235/Land_type_area_ha.csv",
                                         path_to_old_mapping = "../example_outputs/basins235/MOIRAI_land_types.csv",
                                         path_to_new_mapping = "../example_outputs/basins235/MOIRAI_land_types.csv",
                                          error_tolerance= 0.01,
                                         create_land_plot=TRUE,
                                         print_difference_stats=TRUE,
                                         absolute_diff_tolerance=3,
                                         percent_diff_tolerance=0.5){
  
  
  #Add links to datasets
  #1. The original dataset
  Original_LDS_Data<-read.csv(path_to_Original_LDS_Data,skip = 5, stringsAsFactors=FALSE)
  #2. Updated dataset
  Updated_LDS_Data<-read.csv(path_to_Updated_LDS_Data,skip = 5, stringsAsFactors=FALSE)
  #3. Old land types
  MOIRAI_Types_old<-read.csv(path_to_old_mapping,skip=4, stringsAsFactors=FALSE)
  #4. New land types
  MOIRAI_Types_new<-read.csv(path_to_new_mapping,skip = 4, stringsAsFactors=FALSE)
  
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
  
  if(create_land_plot==TRUE){
  g<-ggplot(data=tmp1,aes(x=value,y=Updated_Value))+
    geom_point()+
    ggtitle("Comparing LDS outputs between old and new across all years")+
    labs(subtitle = "This is a comparison at the hyde level i.e Year+ISO+GLU+Hyde_type")
  
  plot(g)
  }
  
  if(print_difference_stats==TRUE){
    
    tmp1 %>% filter(abs(Difference)>absolute_diff_tolerance) %>% mutate(Difference_Percent=if_else(is.infinite(Difference_Percent),100,Difference_Percent))->high_value_diff1    
    print(paste0("Highest percent difference above absolute difference tolerance is -  ",max(abs(high_value_diff1$Difference_Percent))))
    
    tmp1 %>% filter(abs(Difference_Percent)>percent_diff_tolerance)->high_percent_diff1
    print(paste0("Highest absolute difference above percent difference tolerance is -  ",max(abs(high_percent_diff1$Difference))))
        
  }
  
    
  return(expect_equal(tmp1$Updated_Value,tmp1$value, tolerance=error_tolerance,info=paste("Differences at the lowest level are not reasonable")))
}


compare_iso_glu_sage_land_data<-function(path_to_Original_LDS_Data = "../example_outputs/basins235/Land_type_area_ha.csv",
                                         path_to_Updated_LDS_Data = "../example_outputs/basins235/Land_type_area_ha.csv",
                                         path_to_old_mapping = "../example_outputs/basins235/MOIRAI_land_types.csv",
                                         path_to_new_mapping = "../example_outputs/basins235/MOIRAI_land_types.csv",
                                         error_tolerance= 0.01,
                                         create_land_plot=TRUE,
                                         print_difference_stats=TRUE,
                                         absolute_diff_tolerance=3,
                                         percent_diff_tolerance=0.5){
  
  
  #Add links to datasets
  #1. The original dataset
  Original_LDS_Data<-read.csv(path_to_Original_LDS_Data,skip = 5, stringsAsFactors=FALSE)
  #2. Updated dataset
  Updated_LDS_Data<-read.csv(path_to_Updated_LDS_Data,skip = 5, stringsAsFactors=FALSE)
  #3. Old land types
  MOIRAI_Types_old<-read.csv(path_to_old_mapping,skip=4, stringsAsFactors=FALSE)
  #4. New land types
  MOIRAI_Types_new<-read.csv(path_to_new_mapping,skip = 4, stringsAsFactors=FALSE)
  
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
  
  
  
  if(print_difference_stats==TRUE){
    
    tmp2 %>% filter(abs(Difference)>absolute_diff_tolerance) %>% mutate(Difference_Percent=if_else(is.infinite(Difference_Percent),100,Difference_Percent))->high_value_diff1    
    print(paste0("Highest percent difference above absolute difference tolerance is -  ",max(abs(high_value_diff1$Difference_Percent))))
    
    tmp2 %>% filter(abs(Difference_Percent)>percent_diff_tolerance)->high_percent_diff1
    print(paste0("Highest absolute difference above percent difference tolerance is -  ",max(abs(high_percent_diff1$Difference))))
    
  }
  
  if(create_land_plot==TRUE){
    
    ISO_GLU_SAGE_Year_Data_Old %>% 
            full_join(ISO_GLU_SAGE_Year_Data_New,by=c("iso","glu_code","LT_SAGE","year")) %>% 
            mutate(Updated_Value= if_else(is.na(Updated_Value),0,Updated_Value)) %>%
            mutate(value= if_else(is.na(value),0,value)) ->plot_data
           
    
    
    g<-ggplot(data=plot_data,  
              aes(x=value,y=Updated_Value))+
      geom_point()+
      ggtitle("Comparing LDS outputs between old and new across all years")+
      labs(subtitle = "This is a comparison at the sage level i.e Year+ISO+GLU+Sage_type")
    
    plot(g)
  }
  
  
  
  
  return(expect_equal(tmp2$Updated_Value,tmp2$value, tolerance=error_tolerance,info=paste("Differences at the lowest level are not reasonable")))
  
}



compare_iso_glu_hyde_sage_land_data<-function(path_to_Original_LDS_Data = "../example_outputs/basins235/Land_type_area_ha.csv",
                                         path_to_Updated_LDS_Data = "../example_outputs/basins235/Land_type_area_ha.csv",
                                         path_to_old_mapping = "../example_outputs/basins235/MOIRAI_land_types.csv",
                                         path_to_new_mapping = "../example_outputs/basins235/MOIRAI_land_types.csv",
                                         error_tolerance= 0.01,
                                         create_land_plot=TRUE,
                                         print_difference_stats=TRUE,
                                         absolute_diff_tolerance=3,
                                         percent_diff_tolerance=0.5){


#Add links to datasets
#1. The original dataset
Original_LDS_Data<-read.csv(path_to_Original_LDS_Data,skip = 5, stringsAsFactors=FALSE)
#2. Updated dataset
Updated_LDS_Data<-read.csv(path_to_Updated_LDS_Data,skip = 5, stringsAsFactors=FALSE)
#3. Old land types
MOIRAI_Types_old<-read.csv(path_to_old_mapping,skip=4, stringsAsFactors=FALSE)
#4. New land types
MOIRAI_Types_new<-read.csv(path_to_new_mapping,skip = 4, stringsAsFactors=FALSE)


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

if(create_land_plot==TRUE){

g<-ggplot(data=tmp,aes(x=value,y=Updated_Value))+
   geom_point()+
   ggtitle("Comparing LDS outputs between old and new across all years")+
   labs(subtitle = "This is a comparison at the lowest level i.e Year+ISO+GLU+SAGE_type+Hyde_type")

plot(g)

}


if(print_difference_stats==TRUE){
  
  tmp %>% filter(abs(Difference)>absolute_diff_tolerance) %>% mutate(Difference_Percent=if_else(is.infinite(Difference_Percent),100,Difference_Percent))->high_value_diff1    
  print(paste0("Highest percent difference above absolute difference tolerance is -  ",max(abs(high_value_diff1$Difference_Percent))))
  
  tmp %>% filter(abs(Difference_Percent)>percent_diff_tolerance)->high_percent_diff1
  print(paste0("Highest absolute difference above percent difference tolerance is -  ",max(abs(high_percent_diff1$Difference))))
  
}

return(expect_equal(tmp$Updated_Value,tmp$value, tolerance=error_tolerance,info=paste("Differences at the lowest level are not reasonable")))

}







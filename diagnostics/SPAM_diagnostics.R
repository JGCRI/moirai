library(ggplot2)
library(dplyr)
library(rgcam)
library(tidyverse)

# Get mapSPAM and MIRCA harvested area totals ======
mapSPAM_map <- read.csv('mapSPAM_crop_map.csv')

mapSPAM_irr_H_ha <- read.csv('../example_outputs/basins235/mapSPAM_irr_H_ha.csv', skip = 5) %>%
  left_join(mapSPAM_map, join_by('mapspam_crop')) %>%
  dplyr::select(-mapspam_crop) %>%
  mutate(type = 'irr')

mapSPAM_rfd_H_ha <- read.csv('../example_outputs/basins235/mapSPAM_rfd_H_ha.csv', skip = 5) %>%
  left_join(mapSPAM_map, join_by('mapspam_crop')) %>%
  dplyr::select(-mapspam_crop) %>%
  mutate(type = 'rfd')

mapSPAM_H_ha <- bind_rows(mapSPAM_irr_H_ha, mapSPAM_rfd_H_ha)

mapSPAM_H_tot <- mapSPAM_H_ha %>%
  group_by(crop_name, type) %>%
  summarise(value = sum(value))

MIRCA_map <- read.csv('MIRCA_crop_map.csv')

MIRCA_irr_H_ha <- read.csv('../example_outputs/basins235/MIRCA_irrHA_ha.csv', skip = 5) %>%
  left_join(MIRCA_map, join_by('mirca_crop')) %>%
  dplyr::select(-mirca_crop) %>%
  mutate(type = 'irr')

MIRCA_rfd_H_ha <- read.csv('../example_outputs/basins235/MIRCA_rfdHA_ha.csv', skip = 5) %>%
  left_join(MIRCA_map, join_by('mirca_crop')) %>%
  dplyr::select(-mirca_crop) %>%
  mutate(type = 'rfd')

MIRCA_H_ha <- bind_rows(MIRCA_irr_H_ha, MIRCA_rfd_H_ha)

MIRCA_H_tot <- MIRCA_H_ha %>%
  group_by(crop_name, type) %>%
  summarise(value = sum(value))

MIRCA_mapSPAM_map <- read.csv('MIRCA_mapSPAM_crop_map.csv')

Harvested_Area <- mapSPAM_H_tot %>%
  left_join(MIRCA_mapSPAM_map, by = c(crop_name = 'mapspam_crop')) %>%
  ungroup() %>%
  select(-crop_name) %>%
  rename(crop_name = mirca_crop) %>%
  mutate(source = 'mapSPAM') %>%
  bind_rows(MIRCA_H_tot %>%
              mutate(source = 'MIRCA'))

HA_combined <- Harvested_Area %>%
  group_by(type, crop_name, source) %>%
  summarize(value = sum(value))
  
HA_glu_iso <- mapSPAM_H_ha %>%
  group_by(iso, glu_code, type) %>%
  summarize(value_SPAM = sum(value)) %>%
  left_join(MIRCA_H_ha %>%
              group_by(iso, glu_code, type) %>%
              summarize(value_MIRCA = sum(value)), by = c('iso', 'glu_code', 'type'))

# Plot mapSPAM and MIRCA harvested area stats ======
p_mapSPAM_H_tot <- ggplot(mapSPAM_H_tot, aes(crop_name, value, color = type)) +
  geom_point() +
  ggtitle("mapSPAM Harvested Area by Crop") +
  ylab('ha') +
  theme(axis.text.x = element_text(angle = 90, hjust = 1, vjust = 1))
ggsave('mapSPAM_H_tot.png', p_mapSPAM_H_tot)

p_MIRCA_H_tot <- ggplot(MIRCA_H_tot, aes(crop_name, value, color = type)) +
  geom_point() +
  ggtitle("MIRCA Harvested Area by Crop") +
  ylab('ha') +
  theme(axis.text.x = element_text(angle = 90, hjust = 1, vjust = 1))
ggsave('MIRCA_H_tot.png', p_MIRCA_H_tot)

p_H_tot <- ggplot(HA_combined, aes(crop_name, value, color = type, shape = source)) +
  geom_point() +
  ggtitle("Harvested Area by Crop and Source") +
  ylab('ha') +
  theme(axis.text.x = element_text(angle = 90, hjust = 1, vjust = 1))
ggsave('HA_combined_tot.png', p_H_tot)

p_H_iso <- ggplot(HA_glu_iso,aes(x=value_MIRCA,y=value_SPAM, color = type))+
  geom_point()+
  geom_abline(slope = 1, intercept = 0) +
  ggtitle("Comparing total harvested area between MIRCA and mapSPAM across all crops")+
  labs(subtitle = "This is a comparison at the ISO level")
ggsave('HA_glu_iso.png', p_H_iso)

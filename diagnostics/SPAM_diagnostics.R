library(ggplot2)
library(dplyr)
library(rgcam)
library(tidyverse)
library(grid)
library(sf)

# Get mapSPAM and MIRCA harvested area totals ======
mapSPAM_map <- read.csv('mapSPAM_crop_map.csv')
mapSPAM_2020_path <- 'moirai_out/mapSPAM_2020/'
mapSPAM_2000_path <- 'moirai_out/mapSPAM_2000/'

# 2020 mapSPAM data
mapSPAM_irr_H_ha <- read.csv(paste0(mapSPAM_2020_path,'mapSPAM_irr_H_ha.csv'), skip = 5) %>%
  left_join(mapSPAM_map, join_by('mapspam_crop')) %>%
  dplyr::select(-mapspam_crop, -crop_name_2000) %>%
  rename(crop_name = crop_name_2020) %>%
  mutate(type = 'irr')

mapSPAM_rfd_H_ha <- read.csv(paste0(mapSPAM_2020_path,'mapSPAM_rfd_H_ha.csv'), skip = 5) %>%
  left_join(mapSPAM_map, join_by('mapspam_crop')) %>%
  dplyr::select(-mapspam_crop, -crop_name_2000) %>%
  rename(crop_name = crop_name_2020) %>%
  mutate(type = 'rfd')

mapSPAM_H_ha_2020 <- bind_rows(mapSPAM_irr_H_ha, mapSPAM_rfd_H_ha)

mapSPAM_H_tot_2020 <- mapSPAM_H_ha_2020 %>%
  group_by(crop_name, type) %>%
  summarise(value = sum(value))

# 2000 mapSPAM data
mapSPAM_irr_H_ha <- read.csv(paste0(mapSPAM_2000_path,'mapSPAM_irr_H_ha.csv'), skip = 5) %>%
  left_join(mapSPAM_map, join_by('mapspam_crop')) %>%
  dplyr::select(-mapspam_crop, -crop_name_2020) %>%
  rename(crop_name = crop_name_2000) %>%
  mutate(type = 'irr')

mapSPAM_rfd_H_ha <- read.csv(paste0(mapSPAM_2000_path,'mapSPAM_rfd_H_ha.csv'), skip = 5) %>%
  left_join(mapSPAM_map, join_by('mapspam_crop')) %>%
  dplyr::select(-mapspam_crop, -crop_name_2020) %>%
  rename(crop_name = crop_name_2000) %>%
  mutate(type = 'rfd')

mapSPAM_H_ha_2000 <- bind_rows(mapSPAM_irr_H_ha, mapSPAM_rfd_H_ha)

mapSPAM_H_tot_2000 <- mapSPAM_H_ha_2000 %>%
  group_by(crop_name, type) %>%
  summarise(value = sum(value))

# 2000 MIRCA data
MIRCA_map <- read.csv('MIRCA_crop_map.csv')
MIRCA_irr_H_ha <- read.csv('moirai_out/MIRCA_irrHA_ha.csv', skip = 5) %>%
  left_join(MIRCA_map, join_by('mirca_crop')) %>%
  dplyr::select(-mirca_crop) %>%
  mutate(type = 'irr')

MIRCA_rfd_H_ha <- read.csv('moirai_out/MIRCA_rfdHA_ha.csv', skip = 5) %>%
  left_join(MIRCA_map, join_by('mirca_crop')) %>%
  dplyr::select(-mirca_crop) %>%
  mutate(type = 'rfd')

MIRCA_H_ha <- bind_rows(MIRCA_irr_H_ha, MIRCA_rfd_H_ha)

MIRCA_H_tot <- MIRCA_H_ha %>%
  group_by(crop_name, type) %>%
  summarise(value = sum(value))

MIRCA_mapSPAM_map <- read.csv('MIRCA_mapSPAM_crop_map.csv')

# 2020 Harvested Area
Harvested_Area_2020 <- mapSPAM_H_tot_2020 %>%
  left_join(MIRCA_mapSPAM_map, by = c(crop_name = 'mapspam_crop_2020')) %>%
  ungroup() %>%
  select(-crop_name, -mapspam_crop_2000) %>%
  rename(crop_name = mirca_crop) %>%
  mutate(source = 'mapSPAM') %>%
  bind_rows(MIRCA_H_tot %>%
              mutate(source = 'MIRCA'))

HA_combined_2020 <- Harvested_Area_2020 %>%
  group_by(type, crop_name, source) %>%
  summarize(value = sum(value))

HA_glu_iso_2020 <- mapSPAM_H_ha_2020 %>%
  group_by(iso, glu_code, type) %>%
  summarize(value_SPAM = sum(value)) %>%
  left_join(MIRCA_H_ha %>%
              group_by(iso, glu_code, type) %>%
              summarize(value_MIRCA = sum(value)), by = c('iso', 'glu_code', 'type'))

# 2000 Harvested Area
Harvested_Area_2000 <- mapSPAM_H_tot_2000 %>%
  left_join(MIRCA_mapSPAM_map, by = c(crop_name = 'mapspam_crop_2000')) %>%
  ungroup() %>%
  select(-crop_name, -mapspam_crop_2020) %>%
  rename(crop_name = mirca_crop) %>%
  mutate(source = 'mapSPAM') %>%
  bind_rows(MIRCA_H_tot %>%
              mutate(source = 'MIRCA'))

HA_combined_2000 <- Harvested_Area_2000 %>%
  group_by(type, crop_name, source) %>%
  summarize(value = sum(value))

HA_glu_iso_2000 <- mapSPAM_H_ha_2000 %>%
  group_by(iso, glu_code, type) %>%
  summarize(value_SPAM = sum(value)) %>%
  left_join(MIRCA_H_ha %>%
              group_by(iso, glu_code, type) %>%
              summarize(value_MIRCA = sum(value)), by = c('iso', 'glu_code', 'type'))

# Plot MIRCA harvested area stats ======
p_MIRCA_H_tot <- ggplot(MIRCA_H_tot %>%
                          mutate(value = value/1000000), aes(crop_name, value, color = type)) +
  geom_point() +
  ggtitle("MIRCA Harvested Area by Crop") +
  ylab('million ha') +
  theme(axis.text.x = element_text(angle = 90, hjust = 1, vjust = 1))
ggsave('MIRCA_H_tot_2020.png', p_MIRCA_H_tot)

# Plot mapSPAM harvested area stats 2020 ======
p_mapSPAM_H_tot_2020 <- ggplot(mapSPAM_H_tot_2020 %>%
                                 mutate(value = value/1000000), aes(crop_name, value, color = type)) +
  geom_point() +
  ggtitle("mapSPAM 2020 Harvested Area by Crop") +
  ylab('million ha') +
  theme(axis.text.x = element_text(angle = 90, hjust = 1, vjust = 1))
ggsave('mapSPAM_H_tot_2020.png', p_mapSPAM_H_tot_2020)

p_H_tot_2020 <- ggplot(HA_combined_2020 %>%
                         mutate(value = value/1000000), aes(crop_name, value, color = type, shape = source)) +
  geom_point() +
  ggtitle("2020 Harvested Area by Crop and Source") +
  ylab('million ha') +
  theme(axis.text.x = element_text(angle = 90, hjust = 1, vjust = 1))
ggsave('HA_combined_tot_2020.png', p_H_tot_2020)

global_total_H <- HA_glu_iso_2020 %>%
  replace_na(list(value_MIRCA = 0)) %>%
  group_by(type) %>%
  summarize(value_SPAM = sum(value_SPAM)/1000000,
            value_MIRCA = sum(value_MIRCA)/1000000)
# 1. Update the grob generator function to include a table title
create_table_grob <- function(df) {
  gTree(children = gList(
    # Table background box
    rectGrob(gp = gpar(fill = "white", col = "gray60", lwd = 1)),
    
    # Table Title (Centered at the top)
    textGrob("Harvested Area Total (mm ha)", x = 0.50, y = 0.86, just = "center", 
             gp = gpar(font = 2, fontsize = 10, col = "black")),
    
    # Header divider line (Shifted down below the title)
    linesGrob(x = c(0.04, 0.96), y = c(0.70, 0.70), gp = gpar(col = "gray70", lwd = 1)),
    
    # Column Headers (Shifted down to y = 0.58)
    textGrob("Type",        x = 0.08, y = 0.58, just = "left",  gp = gpar(font = 2, fontsize = 8.5)),
    textGrob("mapSPAM", x = 0.54, y = 0.58, just = "right", gp = gpar(font = 2, fontsize = 8.5)),
    textGrob("MIRCA",   x = 0.92, y = 0.58, just = "right", gp = gpar(font = 2, fontsize = 8.5)),
    
    # Row 1 (irr) (Shifted down to y = 0.36)
    textGrob(as.character(df$type[1]),       x = 0.08, y = 0.36, just = "left",  gp = gpar(fontsize = 8.5)),
    textGrob(round(df$value_SPAM[1], 1),  x = 0.54, y = 0.36, just = "right", gp = gpar(fontsize = 8.5)),
    textGrob(round(df$value_MIRCA[1], 1), x = 0.92, y = 0.36, just = "right", gp = gpar(fontsize = 8.5)),
    
    # Row 2 (rfd) (Shifted down to y = 0.16)
    textGrob(as.character(df$type[2]),       x = 0.08, y = 0.16, just = "left",  gp = gpar(fontsize = 8.5)),
    textGrob(round(df$value_SPAM[2], 1),  x = 0.54, y = 0.16, just = "right", gp = gpar(fontsize = 8.5)),
    textGrob(round(df$value_MIRCA[2], 1), x = 0.92, y = 0.16, just = "right", gp = gpar(fontsize = 8.5))
  ))
}

# 2. Regenerate your table grob
my_table_grob <- create_table_grob(global_total_H)

# 3. Re-run your p_H_iso_2020 ggplot script
p_H_iso_2020 <- ggplot(HA_glu_iso_2020 %>%
                         mutate(value_SPAM = value_SPAM/1000000,
                                value_MIRCA = value_MIRCA/1000000), 
                       aes(x = value_MIRCA, y = value_SPAM, color = type)) +
  geom_point() +
  geom_abline(slope = 1, intercept = 0) +
  annotation_custom(
    grob = my_table_grob, 
    xmin = 0, xmax = 10,   # Kept left at 0, expanded right to 10
    ymin = 23, ymax = 35   # Expanded height slightly (from 25 to 23) to comfortably fit the title
  ) +
  ggtitle("Comparing total harvested area between MIRCA and mapSPAM 2020 across all crops") +
  labs(subtitle = "This is a comparison at the ISO level")

ggsave('HA_glu_iso_2020.png', p_H_iso_2020)

# Plot mapSPAM harvested area stats 2000 ======
p_mapSPAM_H_tot_2000 <- ggplot(mapSPAM_H_tot_2000, aes(crop_name, value, color = type)) +
  geom_point() +
  ggtitle("mapSPAM 2000 Harvested Area by Crop") +
  ylab('ha') +
  theme(axis.text.x = element_text(angle = 90, hjust = 1, vjust = 1))
ggsave('mapSPAM_H_tot_2000.png', p_mapSPAM_H_tot_2000)

p_H_tot_2000 <- ggplot(HA_combined_2000, aes(crop_name, value, color = type, shape = source)) +
  geom_point() +
  ggtitle("2000 Harvested Area by Crop and Source") +
  ylab('ha') +
  theme(axis.text.x = element_text(angle = 90, hjust = 1, vjust = 1))
ggsave('HA_combined_tot_2000.png', p_H_tot_2000)

p_H_iso_2000 <- ggplot(HA_glu_iso_2000,aes(x=value_MIRCA,y=value_SPAM, color = type))+
  geom_point()+
  geom_abline(slope = 1, intercept = 0) +
  ggtitle("Comparing total harvested area between MIRCA and mapSPAM 2000 across all crops")+
  labs(subtitle = "This is a comparison at the ISO level")
ggsave('HA_glu_iso_2000.png', p_H_iso_2000)

# Plot major crop maps by GLU ======
major_crops <- mapSPAM_H_ha_2020 %>%
  filter(crop_name %in% c('Wheat', 'Rice', 'Maize')) %>%
  pivot_wider(names_from = crop_name, values_from = value) %>%
  group_by(glu_code, type) %>%
  summarize(Wheat = sum(Wheat),
            Rice = sum(Rice),
            Maize = sum(Maize))

major_crops_irr <- major_crops %>%
  filter(type == 'irr')
major_crops_rfd <- major_crops %>%
  filter(type == 'rfd')

GLU_shp <- st_as_sf(st_read("../GCIMS-Reuse/GCAM_v9_RegGLU_boundaries/GCAM_v9_401RegGLU_boundaries.shp"))
plot(st_geometry(GLU_shp))

combined_shapes <- GLU_shp %>%
  group_by(glu_id, glu_nm) %>%
  summarize(geometry = st_union(geometry))
plot(st_geometry(combined_shapes))

merged_data <- combined_shapes %>%
  left_join(major_crops_irr, by = c("glu_id" = "glu_code"))

ggplot(data = merged_data) +
  geom_sf(aes(fill = Wheat), color = "white", size = 0.2) +
  scale_fill_viridis_c() + # Use _c() for continuous data, _d() for discrete categories
  theme_minimal() +
  labs(
    title = "Wheat",
    fill = "Harvested Area"
  )

ggplot(data = merged_data) +
  geom_sf(aes(fill = Rice), color = "white", size = 0.2) +
  scale_fill_viridis_c() + # Use _c() for continuous data, _d() for discrete categories
  theme_minimal() +
  labs(
    title = "Rice",
    fill = "Harvested Area"
  )

ggplot(data = merged_data) +
  geom_sf(aes(fill = Maize), color = "white", size = 0.2) +
  scale_fill_viridis_c() + # Use _c() for continuous data, _d() for discrete categories
  theme_minimal() +
  labs(
    title = "Maize",
    fill = "Harvested Area"
  )

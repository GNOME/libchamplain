<?xml version="1.0"?>
<api version="1.0">
	<namespace name="Champlain">
		<callback name="ChamplainMapSourceConstructor">
			<return-type type="ChamplainMapSource*"/>
			<parameters>
				<parameter name="desc" type="ChamplainMapSourceDesc*"/>
				<parameter name="data" type="gpointer"/>
			</parameters>
		</callback>
		<boxed name="ChamplainBoundingBox" type-name="ChamplainBoundingBox" get-type="champlain_bounding_box_get_type">
			<method name="copy" symbol="champlain_bounding_box_copy">
				<return-type type="ChamplainBoundingBox*"/>
				<parameters>
					<parameter name="bbox" type="ChamplainBoundingBox*"/>
				</parameters>
			</method>
			<method name="free" symbol="champlain_bounding_box_free">
				<return-type type="void"/>
				<parameters>
					<parameter name="bbox" type="ChamplainBoundingBox*"/>
				</parameters>
			</method>
			<method name="get_center" symbol="champlain_bounding_box_get_center">
				<return-type type="void"/>
				<parameters>
					<parameter name="bbox" type="ChamplainBoundingBox*"/>
					<parameter name="lat" type="gdouble*"/>
					<parameter name="lon" type="gdouble*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="champlain_bounding_box_new">
				<return-type type="ChamplainBoundingBox*"/>
			</constructor>
			<field name="left" type="gdouble"/>
			<field name="bottom" type="gdouble"/>
			<field name="right" type="gdouble"/>
			<field name="top" type="gdouble"/>
		</boxed>
		<boxed name="ChamplainMapSourceDesc" type-name="ChamplainMapSourceDesc" get-type="champlain_map_source_desc_get_type">
			<method name="copy" symbol="champlain_map_source_desc_copy">
				<return-type type="ChamplainMapSourceDesc*"/>
				<parameters>
					<parameter name="desc" type="ChamplainMapSourceDesc*"/>
				</parameters>
			</method>
			<method name="free" symbol="champlain_map_source_desc_free">
				<return-type type="void"/>
				<parameters>
					<parameter name="desc" type="ChamplainMapSourceDesc*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="champlain_map_source_desc_new">
				<return-type type="ChamplainMapSourceDesc*"/>
			</constructor>
			<field name="id" type="gchar*"/>
			<field name="name" type="gchar*"/>
			<field name="license" type="gchar*"/>
			<field name="license_uri" type="gchar*"/>
			<field name="min_zoom_level" type="gint"/>
			<field name="max_zoom_level" type="gint"/>
			<field name="projection" type="ChamplainMapProjection"/>
			<field name="constructor" type="ChamplainMapSourceConstructor"/>
			<field name="uri_format" type="gchar*"/>
			<field name="data" type="gpointer"/>
		</boxed>
		<boxed name="ChamplainPoint" type-name="ChamplainPoint" get-type="champlain_point_get_type">
			<method name="copy" symbol="champlain_point_copy">
				<return-type type="ChamplainPoint*"/>
				<parameters>
					<parameter name="point" type="ChamplainPoint*"/>
				</parameters>
			</method>
			<method name="free" symbol="champlain_point_free">
				<return-type type="void"/>
				<parameters>
					<parameter name="point" type="ChamplainPoint*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="champlain_point_new">
				<return-type type="ChamplainPoint*"/>
				<parameters>
					<parameter name="lat" type="gdouble"/>
					<parameter name="lon" type="gdouble"/>
				</parameters>
			</constructor>
			<field name="lat" type="double"/>
			<field name="lon" type="double"/>
		</boxed>
		<enum name="ChamplainMapProjection" type-name="ChamplainMapProjection" get-type="champlain_map_projection_get_type">
			<member name="CHAMPLAIN_MAP_PROJECTION_MERCATOR" value="0"/>
		</enum>
		<enum name="ChamplainScrollMode" type-name="ChamplainScrollMode" get-type="champlain_scroll_mode_get_type">
			<member name="CHAMPLAIN_SCROLL_MODE_PUSH" value="0"/>
			<member name="CHAMPLAIN_SCROLL_MODE_KINETIC" value="1"/>
		</enum>
		<enum name="ChamplainSelectionMode" type-name="ChamplainSelectionMode" get-type="champlain_selection_mode_get_type">
			<member name="CHAMPLAIN_SELECTION_NONE" value="0"/>
			<member name="CHAMPLAIN_SELECTION_SINGLE" value="1"/>
			<member name="CHAMPLAIN_SELECTION_MULTIPLE" value="2"/>
		</enum>
		<enum name="ChamplainState" type-name="ChamplainState" get-type="champlain_state_get_type">
			<member name="CHAMPLAIN_STATE_NONE" value="0"/>
			<member name="CHAMPLAIN_STATE_LOADING" value="1"/>
			<member name="CHAMPLAIN_STATE_DONE" value="2"/>
		</enum>
		<enum name="ChamplainUnit" type-name="ChamplainUnit" get-type="champlain_unit_get_type">
			<member name="CHAMPLAIN_UNIT_KM" value="0"/>
			<member name="CHAMPLAIN_UNIT_MILES" value="1"/>
		</enum>
		<object name="ChamplainBaseMarker" parent="ClutterGroup" type-name="ChamplainBaseMarker" get-type="champlain_base_marker_get_type">
			<implements>
				<interface name="ClutterScriptable"/>
				<interface name="ClutterContainer"/>
			</implements>
			<method name="animate_in" symbol="champlain_base_marker_animate_in">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainBaseMarker*"/>
				</parameters>
			</method>
			<method name="animate_in_with_delay" symbol="champlain_base_marker_animate_in_with_delay">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainBaseMarker*"/>
					<parameter name="delay" type="guint"/>
				</parameters>
			</method>
			<method name="animate_out" symbol="champlain_base_marker_animate_out">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainBaseMarker*"/>
				</parameters>
			</method>
			<method name="animate_out_with_delay" symbol="champlain_base_marker_animate_out_with_delay">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainBaseMarker*"/>
					<parameter name="delay" type="guint"/>
				</parameters>
			</method>
			<method name="get_highlighted" symbol="champlain_base_marker_get_highlighted">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="marker" type="ChamplainBaseMarker*"/>
				</parameters>
			</method>
			<method name="get_latitude" symbol="champlain_base_marker_get_latitude">
				<return-type type="gdouble"/>
				<parameters>
					<parameter name="marker" type="ChamplainBaseMarker*"/>
				</parameters>
			</method>
			<method name="get_longitude" symbol="champlain_base_marker_get_longitude">
				<return-type type="gdouble"/>
				<parameters>
					<parameter name="marker" type="ChamplainBaseMarker*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="champlain_base_marker_new">
				<return-type type="ClutterActor*"/>
			</constructor>
			<method name="set_highlighted" symbol="champlain_base_marker_set_highlighted">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainBaseMarker*"/>
					<parameter name="value" type="gboolean"/>
				</parameters>
			</method>
			<method name="set_position" symbol="champlain_base_marker_set_position">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainBaseMarker*"/>
					<parameter name="latitude" type="gdouble"/>
					<parameter name="longitude" type="gdouble"/>
				</parameters>
			</method>
			<property name="highlighted" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="latitude" type="gdouble" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="longitude" type="gdouble" readable="1" writable="1" construct="0" construct-only="0"/>
		</object>
		<object name="ChamplainErrorTileSource" parent="ChamplainTileSource" type-name="ChamplainErrorTileSource" get-type="champlain_error_tile_source_get_type">
			<constructor name="new_full" symbol="champlain_error_tile_source_new_full">
				<return-type type="ChamplainErrorTileSource*"/>
				<parameters>
					<parameter name="tile_size" type="guint"/>
				</parameters>
			</constructor>
		</object>
		<object name="ChamplainFileCache" parent="ChamplainTileCache" type-name="ChamplainFileCache" get-type="champlain_file_cache_get_type">
			<method name="get_cache_dir" symbol="champlain_file_cache_get_cache_dir">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="file_cache" type="ChamplainFileCache*"/>
				</parameters>
			</method>
			<method name="get_size_limit" symbol="champlain_file_cache_get_size_limit">
				<return-type type="guint"/>
				<parameters>
					<parameter name="file_cache" type="ChamplainFileCache*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="champlain_file_cache_new">
				<return-type type="ChamplainFileCache*"/>
			</constructor>
			<constructor name="new_full" symbol="champlain_file_cache_new_full">
				<return-type type="ChamplainFileCache*"/>
				<parameters>
					<parameter name="size_limit" type="guint"/>
					<parameter name="cache_dir" type="gchar*"/>
					<parameter name="persistent" type="gboolean"/>
				</parameters>
			</constructor>
			<method name="purge" symbol="champlain_file_cache_purge">
				<return-type type="void"/>
				<parameters>
					<parameter name="file_cache" type="ChamplainFileCache*"/>
				</parameters>
			</method>
			<method name="purge_on_idle" symbol="champlain_file_cache_purge_on_idle">
				<return-type type="void"/>
				<parameters>
					<parameter name="file_cache" type="ChamplainFileCache*"/>
				</parameters>
			</method>
			<method name="set_size_limit" symbol="champlain_file_cache_set_size_limit">
				<return-type type="void"/>
				<parameters>
					<parameter name="file_cache" type="ChamplainFileCache*"/>
					<parameter name="size_limit" type="guint"/>
				</parameters>
			</method>
			<property name="cache-dir" type="char*" readable="1" writable="1" construct="0" construct-only="1"/>
			<property name="size-limit" type="guint" readable="1" writable="1" construct="1" construct-only="0"/>
		</object>
		<object name="ChamplainLayer" parent="ClutterGroup" type-name="ChamplainLayer" get-type="champlain_layer_get_type">
			<implements>
				<interface name="ClutterScriptable"/>
				<interface name="ClutterContainer"/>
			</implements>
			<method name="add_marker" symbol="champlain_layer_add_marker">
				<return-type type="void"/>
				<parameters>
					<parameter name="layer" type="ChamplainLayer*"/>
					<parameter name="marker" type="ChamplainBaseMarker*"/>
				</parameters>
			</method>
			<method name="animate_in_all_markers" symbol="champlain_layer_animate_in_all_markers">
				<return-type type="void"/>
				<parameters>
					<parameter name="layer" type="ChamplainLayer*"/>
				</parameters>
			</method>
			<method name="animate_out_all_markers" symbol="champlain_layer_animate_out_all_markers">
				<return-type type="void"/>
				<parameters>
					<parameter name="layer" type="ChamplainLayer*"/>
				</parameters>
			</method>
			<method name="hide" symbol="champlain_layer_hide">
				<return-type type="void"/>
				<parameters>
					<parameter name="layer" type="ChamplainLayer*"/>
				</parameters>
			</method>
			<method name="hide_all_markers" symbol="champlain_layer_hide_all_markers">
				<return-type type="void"/>
				<parameters>
					<parameter name="layer" type="ChamplainLayer*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="champlain_layer_new">
				<return-type type="ChamplainLayer*"/>
			</constructor>
			<method name="remove_marker" symbol="champlain_layer_remove_marker">
				<return-type type="void"/>
				<parameters>
					<parameter name="layer" type="ChamplainLayer*"/>
					<parameter name="marker" type="ChamplainBaseMarker*"/>
				</parameters>
			</method>
			<method name="show" symbol="champlain_layer_show">
				<return-type type="void"/>
				<parameters>
					<parameter name="layer" type="ChamplainLayer*"/>
				</parameters>
			</method>
			<method name="show_all_markers" symbol="champlain_layer_show_all_markers">
				<return-type type="void"/>
				<parameters>
					<parameter name="layer" type="ChamplainLayer*"/>
				</parameters>
			</method>
		</object>
		<object name="ChamplainLocalMapDataSource" parent="ChamplainMapDataSource" type-name="ChamplainLocalMapDataSource" get-type="champlain_local_map_data_source_get_type">
			<method name="load_map_data" symbol="champlain_local_map_data_source_load_map_data">
				<return-type type="void"/>
				<parameters>
					<parameter name="map_data_source" type="ChamplainLocalMapDataSource*"/>
					<parameter name="map_path" type="gchar*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="champlain_local_map_data_source_new">
				<return-type type="ChamplainLocalMapDataSource*"/>
			</constructor>
		</object>
		<object name="ChamplainMapDataSource" parent="GInitiallyUnowned" type-name="ChamplainMapDataSource" get-type="champlain_map_data_source_get_type">
			<method name="get_map_data" symbol="champlain_map_data_source_get_map_data">
				<return-type type="MemphisMap*"/>
				<parameters>
					<parameter name="data_source" type="ChamplainMapDataSource*"/>
				</parameters>
			</method>
			<property name="bounding-box" type="ChamplainBoundingBox*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="state" type="ChamplainState" readable="1" writable="1" construct="0" construct-only="0"/>
			<vfunc name="get_map_data">
				<return-type type="MemphisMap*"/>
				<parameters>
					<parameter name="data_source" type="ChamplainMapDataSource*"/>
				</parameters>
			</vfunc>
		</object>
		<object name="ChamplainMapSource" parent="GInitiallyUnowned" type-name="ChamplainMapSource" get-type="champlain_map_source_get_type">
			<method name="fill_tile" symbol="champlain_map_source_fill_tile">
				<return-type type="void"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
					<parameter name="tile" type="ChamplainTile*"/>
				</parameters>
			</method>
			<method name="get_column_count" symbol="champlain_map_source_get_column_count">
				<return-type type="guint"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
					<parameter name="zoom_level" type="guint"/>
				</parameters>
			</method>
			<method name="get_id" symbol="champlain_map_source_get_id">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
				</parameters>
			</method>
			<method name="get_latitude" symbol="champlain_map_source_get_latitude">
				<return-type type="gdouble"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
					<parameter name="zoom_level" type="guint"/>
					<parameter name="y" type="guint"/>
				</parameters>
			</method>
			<method name="get_license" symbol="champlain_map_source_get_license">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
				</parameters>
			</method>
			<method name="get_license_uri" symbol="champlain_map_source_get_license_uri">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
				</parameters>
			</method>
			<method name="get_longitude" symbol="champlain_map_source_get_longitude">
				<return-type type="gdouble"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
					<parameter name="zoom_level" type="guint"/>
					<parameter name="x" type="guint"/>
				</parameters>
			</method>
			<method name="get_max_zoom_level" symbol="champlain_map_source_get_max_zoom_level">
				<return-type type="guint"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
				</parameters>
			</method>
			<method name="get_meters_per_pixel" symbol="champlain_map_source_get_meters_per_pixel">
				<return-type type="gdouble"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
					<parameter name="zoom_level" type="guint"/>
					<parameter name="latitude" type="gdouble"/>
					<parameter name="longitude" type="gdouble"/>
				</parameters>
			</method>
			<method name="get_min_zoom_level" symbol="champlain_map_source_get_min_zoom_level">
				<return-type type="guint"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
				</parameters>
			</method>
			<method name="get_name" symbol="champlain_map_source_get_name">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
				</parameters>
			</method>
			<method name="get_next_source" symbol="champlain_map_source_get_next_source">
				<return-type type="ChamplainMapSource*"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
				</parameters>
			</method>
			<method name="get_projection" symbol="champlain_map_source_get_projection">
				<return-type type="ChamplainMapProjection"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
				</parameters>
			</method>
			<method name="get_row_count" symbol="champlain_map_source_get_row_count">
				<return-type type="guint"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
					<parameter name="zoom_level" type="guint"/>
				</parameters>
			</method>
			<method name="get_tile_size" symbol="champlain_map_source_get_tile_size">
				<return-type type="guint"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
				</parameters>
			</method>
			<method name="get_x" symbol="champlain_map_source_get_x">
				<return-type type="guint"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
					<parameter name="zoom_level" type="guint"/>
					<parameter name="longitude" type="gdouble"/>
				</parameters>
			</method>
			<method name="get_y" symbol="champlain_map_source_get_y">
				<return-type type="guint"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
					<parameter name="zoom_level" type="guint"/>
					<parameter name="latitude" type="gdouble"/>
				</parameters>
			</method>
			<method name="set_next_source" symbol="champlain_map_source_set_next_source">
				<return-type type="void"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
					<parameter name="next_source" type="ChamplainMapSource*"/>
				</parameters>
			</method>
			<property name="next-source" type="ChamplainMapSource*" readable="1" writable="1" construct="0" construct-only="0"/>
			<signal name="reload-tiles" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="ChamplainMapSource*"/>
				</parameters>
			</signal>
			<vfunc name="fill_tile">
				<return-type type="void"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
					<parameter name="tile" type="ChamplainTile*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_id">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_license">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_license_uri">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_max_zoom_level">
				<return-type type="guint"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_min_zoom_level">
				<return-type type="guint"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_name">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_projection">
				<return-type type="ChamplainMapProjection"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
				</parameters>
			</vfunc>
			<vfunc name="get_tile_size">
				<return-type type="guint"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
				</parameters>
			</vfunc>
			<vfunc name="on_set_next_source">
				<return-type type="void"/>
				<parameters>
					<parameter name="map_source" type="ChamplainMapSource*"/>
					<parameter name="next_source" type="ChamplainMapSource*"/>
					<parameter name="new_next_source" type="ChamplainMapSource*"/>
				</parameters>
			</vfunc>
		</object>
		<object name="ChamplainMapSourceChain" parent="ChamplainMapSource" type-name="ChamplainMapSourceChain" get-type="champlain_map_source_chain_get_type">
			<constructor name="new" symbol="champlain_map_source_chain_new">
				<return-type type="ChamplainMapSourceChain*"/>
			</constructor>
			<method name="pop" symbol="champlain_map_source_chain_pop">
				<return-type type="void"/>
				<parameters>
					<parameter name="source_chain" type="ChamplainMapSourceChain*"/>
				</parameters>
			</method>
			<method name="push" symbol="champlain_map_source_chain_push">
				<return-type type="void"/>
				<parameters>
					<parameter name="source_chain" type="ChamplainMapSourceChain*"/>
					<parameter name="map_source" type="ChamplainMapSource*"/>
				</parameters>
			</method>
		</object>
		<object name="ChamplainMapSourceFactory" parent="GObject" type-name="ChamplainMapSourceFactory" get-type="champlain_map_source_factory_get_type">
			<method name="create" symbol="champlain_map_source_factory_create">
				<return-type type="ChamplainMapSource*"/>
				<parameters>
					<parameter name="factory" type="ChamplainMapSourceFactory*"/>
					<parameter name="id" type="gchar*"/>
				</parameters>
			</method>
			<method name="create_cached_source" symbol="champlain_map_source_factory_create_cached_source">
				<return-type type="ChamplainMapSource*"/>
				<parameters>
					<parameter name="factory" type="ChamplainMapSourceFactory*"/>
					<parameter name="id" type="gchar*"/>
				</parameters>
			</method>
			<method name="dup_default" symbol="champlain_map_source_factory_dup_default">
				<return-type type="ChamplainMapSourceFactory*"/>
			</method>
			<method name="dup_list" symbol="champlain_map_source_factory_dup_list">
				<return-type type="GSList*"/>
				<parameters>
					<parameter name="factory" type="ChamplainMapSourceFactory*"/>
				</parameters>
			</method>
			<method name="register" symbol="champlain_map_source_factory_register">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="factory" type="ChamplainMapSourceFactory*"/>
					<parameter name="desc" type="ChamplainMapSourceDesc*"/>
					<parameter name="constructor" type="ChamplainMapSourceConstructor"/>
					<parameter name="data" type="gpointer"/>
				</parameters>
			</method>
		</object>
		<object name="ChamplainMarker" parent="ChamplainBaseMarker" type-name="ChamplainMarker" get-type="champlain_marker_get_type">
			<implements>
				<interface name="ClutterScriptable"/>
				<interface name="ClutterContainer"/>
			</implements>
			<method name="get_alignment" symbol="champlain_marker_get_alignment">
				<return-type type="PangoAlignment"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
				</parameters>
			</method>
			<method name="get_color" symbol="champlain_marker_get_color">
				<return-type type="ClutterColor*"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
				</parameters>
			</method>
			<method name="get_draw_background" symbol="champlain_marker_get_draw_background">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
				</parameters>
			</method>
			<method name="get_ellipsize" symbol="champlain_marker_get_ellipsize">
				<return-type type="PangoEllipsizeMode"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
				</parameters>
			</method>
			<method name="get_font_name" symbol="champlain_marker_get_font_name">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
				</parameters>
			</method>
			<method name="get_highlight_color" symbol="champlain_marker_get_highlight_color">
				<return-type type="ClutterColor*"/>
			</method>
			<method name="get_highlight_text_color" symbol="champlain_marker_get_highlight_text_color">
				<return-type type="ClutterColor*"/>
			</method>
			<method name="get_image" symbol="champlain_marker_get_image">
				<return-type type="ClutterActor*"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
				</parameters>
			</method>
			<method name="get_single_line_mode" symbol="champlain_marker_get_single_line_mode">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
				</parameters>
			</method>
			<method name="get_text" symbol="champlain_marker_get_text">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
				</parameters>
			</method>
			<method name="get_text_color" symbol="champlain_marker_get_text_color">
				<return-type type="ClutterColor*"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
				</parameters>
			</method>
			<method name="get_use_markup" symbol="champlain_marker_get_use_markup">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
				</parameters>
			</method>
			<method name="get_wrap" symbol="champlain_marker_get_wrap">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
				</parameters>
			</method>
			<method name="get_wrap_mode" symbol="champlain_marker_get_wrap_mode">
				<return-type type="PangoWrapMode"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="champlain_marker_new">
				<return-type type="ClutterActor*"/>
			</constructor>
			<constructor name="new_from_file" symbol="champlain_marker_new_from_file">
				<return-type type="ClutterActor*"/>
				<parameters>
					<parameter name="filename" type="gchar*"/>
					<parameter name="error" type="GError**"/>
				</parameters>
			</constructor>
			<constructor name="new_full" symbol="champlain_marker_new_full">
				<return-type type="ClutterActor*"/>
				<parameters>
					<parameter name="text" type="gchar*"/>
					<parameter name="actor" type="ClutterActor*"/>
				</parameters>
			</constructor>
			<constructor name="new_with_image" symbol="champlain_marker_new_with_image">
				<return-type type="ClutterActor*"/>
				<parameters>
					<parameter name="actor" type="ClutterActor*"/>
				</parameters>
			</constructor>
			<constructor name="new_with_text" symbol="champlain_marker_new_with_text">
				<return-type type="ClutterActor*"/>
				<parameters>
					<parameter name="text" type="gchar*"/>
					<parameter name="font" type="gchar*"/>
					<parameter name="text_color" type="ClutterColor*"/>
					<parameter name="marker_color" type="ClutterColor*"/>
				</parameters>
			</constructor>
			<method name="queue_redraw" symbol="champlain_marker_queue_redraw">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
				</parameters>
			</method>
			<method name="set_alignment" symbol="champlain_marker_set_alignment">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
					<parameter name="alignment" type="PangoAlignment"/>
				</parameters>
			</method>
			<method name="set_attributes" symbol="champlain_marker_set_attributes">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
					<parameter name="list" type="PangoAttrList*"/>
				</parameters>
			</method>
			<method name="set_color" symbol="champlain_marker_set_color">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
					<parameter name="color" type="ClutterColor*"/>
				</parameters>
			</method>
			<method name="set_draw_background" symbol="champlain_marker_set_draw_background">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
					<parameter name="background" type="gboolean"/>
				</parameters>
			</method>
			<method name="set_ellipsize" symbol="champlain_marker_set_ellipsize">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
					<parameter name="mode" type="PangoEllipsizeMode"/>
				</parameters>
			</method>
			<method name="set_font_name" symbol="champlain_marker_set_font_name">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
					<parameter name="font_name" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_highlight_color" symbol="champlain_marker_set_highlight_color">
				<return-type type="void"/>
				<parameters>
					<parameter name="color" type="ClutterColor*"/>
				</parameters>
			</method>
			<method name="set_highlight_text_color" symbol="champlain_marker_set_highlight_text_color">
				<return-type type="void"/>
				<parameters>
					<parameter name="color" type="ClutterColor*"/>
				</parameters>
			</method>
			<method name="set_image" symbol="champlain_marker_set_image">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
					<parameter name="image" type="ClutterActor*"/>
				</parameters>
			</method>
			<method name="set_single_line_mode" symbol="champlain_marker_set_single_line_mode">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
					<parameter name="mode" type="gboolean"/>
				</parameters>
			</method>
			<method name="set_text" symbol="champlain_marker_set_text">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
					<parameter name="text" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_text_color" symbol="champlain_marker_set_text_color">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
					<parameter name="color" type="ClutterColor*"/>
				</parameters>
			</method>
			<method name="set_use_markup" symbol="champlain_marker_set_use_markup">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
					<parameter name="use_markup" type="gboolean"/>
				</parameters>
			</method>
			<method name="set_wrap" symbol="champlain_marker_set_wrap">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
					<parameter name="wrap" type="gboolean"/>
				</parameters>
			</method>
			<method name="set_wrap_mode" symbol="champlain_marker_set_wrap_mode">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
					<parameter name="wrap_mode" type="PangoWrapMode"/>
				</parameters>
			</method>
			<property name="alignment" type="PangoAlignment" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="color" type="ClutterColor*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="draw-background" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="ellipsize" type="PangoEllipsizeMode" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="font-name" type="char*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="image" type="ClutterActor*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="single-line-mode" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="text" type="char*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="text-color" type="ClutterColor*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="use-markup" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="wrap" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="wrap-mode" type="PangoWrapMode" readable="1" writable="1" construct="0" construct-only="0"/>
			<vfunc name="draw_marker">
				<return-type type="void"/>
				<parameters>
					<parameter name="marker" type="ChamplainMarker*"/>
				</parameters>
			</vfunc>
		</object>
		<object name="ChamplainMemphisTileSource" parent="ChamplainTileSource" type-name="ChamplainMemphisTileSource" get-type="champlain_memphis_tile_source_get_type">
			<method name="get_background_color" symbol="champlain_memphis_tile_source_get_background_color">
				<return-type type="ClutterColor*"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainMemphisTileSource*"/>
				</parameters>
			</method>
			<method name="get_map_data_source" symbol="champlain_memphis_tile_source_get_map_data_source">
				<return-type type="ChamplainMapDataSource*"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainMemphisTileSource*"/>
				</parameters>
			</method>
			<method name="get_rule" symbol="champlain_memphis_tile_source_get_rule">
				<return-type type="MemphisRule*"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainMemphisTileSource*"/>
					<parameter name="id" type="gchar*"/>
				</parameters>
			</method>
			<method name="get_rule_ids" symbol="champlain_memphis_tile_source_get_rule_ids">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainMemphisTileSource*"/>
				</parameters>
			</method>
			<method name="load_rules" symbol="champlain_memphis_tile_source_load_rules">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainMemphisTileSource*"/>
					<parameter name="rules_path" type="gchar*"/>
				</parameters>
			</method>
			<constructor name="new_full" symbol="champlain_memphis_tile_source_new_full">
				<return-type type="ChamplainMemphisTileSource*"/>
				<parameters>
					<parameter name="id" type="gchar*"/>
					<parameter name="name" type="gchar*"/>
					<parameter name="license" type="gchar*"/>
					<parameter name="license_uri" type="gchar*"/>
					<parameter name="min_zoom" type="guint"/>
					<parameter name="max_zoom" type="guint"/>
					<parameter name="tile_size" type="guint"/>
					<parameter name="projection" type="ChamplainMapProjection"/>
					<parameter name="map_data_source" type="ChamplainMapDataSource*"/>
				</parameters>
			</constructor>
			<method name="remove_rule" symbol="champlain_memphis_tile_source_remove_rule">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainMemphisTileSource*"/>
					<parameter name="id" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_background_color" symbol="champlain_memphis_tile_source_set_background_color">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainMemphisTileSource*"/>
					<parameter name="color" type="ClutterColor*"/>
				</parameters>
			</method>
			<method name="set_map_data_source" symbol="champlain_memphis_tile_source_set_map_data_source">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainMemphisTileSource*"/>
					<parameter name="map_data_source" type="ChamplainMapDataSource*"/>
				</parameters>
			</method>
			<method name="set_rule" symbol="champlain_memphis_tile_source_set_rule">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainMemphisTileSource*"/>
					<parameter name="rule" type="MemphisRule*"/>
				</parameters>
			</method>
			<property name="map-data-source" type="ChamplainMapDataSource*" readable="1" writable="1" construct="1" construct-only="0"/>
		</object>
		<object name="ChamplainNetworkMapDataSource" parent="ChamplainMapDataSource" type-name="ChamplainNetworkMapDataSource" get-type="champlain_network_map_data_source_get_type">
			<method name="get_api_uri" symbol="champlain_network_map_data_source_get_api_uri">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="map_data_source" type="ChamplainNetworkMapDataSource*"/>
				</parameters>
			</method>
			<method name="load_map_data" symbol="champlain_network_map_data_source_load_map_data">
				<return-type type="void"/>
				<parameters>
					<parameter name="map_data_source" type="ChamplainNetworkMapDataSource*"/>
					<parameter name="bound_left" type="gdouble"/>
					<parameter name="bound_bottom" type="gdouble"/>
					<parameter name="bound_right" type="gdouble"/>
					<parameter name="bound_top" type="gdouble"/>
				</parameters>
			</method>
			<constructor name="new" symbol="champlain_network_map_data_source_new">
				<return-type type="ChamplainNetworkMapDataSource*"/>
			</constructor>
			<method name="set_api_uri" symbol="champlain_network_map_data_source_set_api_uri">
				<return-type type="void"/>
				<parameters>
					<parameter name="map_data_source" type="ChamplainNetworkMapDataSource*"/>
					<parameter name="api_uri" type="gchar*"/>
				</parameters>
			</method>
			<property name="api-uri" type="char*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="proxy-uri" type="char*" readable="1" writable="1" construct="0" construct-only="0"/>
		</object>
		<object name="ChamplainNetworkTileSource" parent="ChamplainTileSource" type-name="ChamplainNetworkTileSource" get-type="champlain_network_tile_source_get_type">
			<method name="get_offline" symbol="champlain_network_tile_source_get_offline">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainNetworkTileSource*"/>
				</parameters>
			</method>
			<method name="get_proxy_uri" symbol="champlain_network_tile_source_get_proxy_uri">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainNetworkTileSource*"/>
				</parameters>
			</method>
			<method name="get_uri_format" symbol="champlain_network_tile_source_get_uri_format">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainNetworkTileSource*"/>
				</parameters>
			</method>
			<constructor name="new_full" symbol="champlain_network_tile_source_new_full">
				<return-type type="ChamplainNetworkTileSource*"/>
				<parameters>
					<parameter name="id" type="gchar*"/>
					<parameter name="name" type="gchar*"/>
					<parameter name="license" type="gchar*"/>
					<parameter name="license_uri" type="gchar*"/>
					<parameter name="min_zoom" type="guint"/>
					<parameter name="max_zoom" type="guint"/>
					<parameter name="tile_size" type="guint"/>
					<parameter name="projection" type="ChamplainMapProjection"/>
					<parameter name="uri_format" type="gchar*"/>
				</parameters>
			</constructor>
			<method name="set_offline" symbol="champlain_network_tile_source_set_offline">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainNetworkTileSource*"/>
					<parameter name="offline" type="gboolean"/>
				</parameters>
			</method>
			<method name="set_proxy_uri" symbol="champlain_network_tile_source_set_proxy_uri">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainNetworkTileSource*"/>
					<parameter name="proxy_uri" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_uri_format" symbol="champlain_network_tile_source_set_uri_format">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainNetworkTileSource*"/>
					<parameter name="uri_format" type="gchar*"/>
				</parameters>
			</method>
			<property name="offline" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="proxy-uri" type="char*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="uri-format" type="char*" readable="1" writable="1" construct="1" construct-only="0"/>
		</object>
		<object name="ChamplainPolygon" parent="ClutterGroup" type-name="ChamplainPolygon" get-type="champlain_polygon_get_type">
			<implements>
				<interface name="ClutterScriptable"/>
				<interface name="ClutterContainer"/>
			</implements>
			<method name="append_point" symbol="champlain_polygon_append_point">
				<return-type type="ChamplainPoint*"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
					<parameter name="lat" type="gdouble"/>
					<parameter name="lon" type="gdouble"/>
				</parameters>
			</method>
			<method name="clear_points" symbol="champlain_polygon_clear_points">
				<return-type type="void"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
				</parameters>
			</method>
			<method name="draw_polygon" symbol="champlain_polygon_draw_polygon">
				<return-type type="void"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
					<parameter name="map_source" type="ChamplainMapSource*"/>
					<parameter name="zoom_level" type="guint"/>
					<parameter name="width" type="gfloat"/>
					<parameter name="height" type="gfloat"/>
					<parameter name="shift_x" type="gfloat"/>
					<parameter name="shift_y" type="gfloat"/>
				</parameters>
			</method>
			<method name="get_fill" symbol="champlain_polygon_get_fill">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
				</parameters>
			</method>
			<method name="get_fill_color" symbol="champlain_polygon_get_fill_color">
				<return-type type="ClutterColor*"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
				</parameters>
			</method>
			<method name="get_mark_points" symbol="champlain_polygon_get_mark_points">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
				</parameters>
			</method>
			<method name="get_points" symbol="champlain_polygon_get_points">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
				</parameters>
			</method>
			<method name="get_stroke" symbol="champlain_polygon_get_stroke">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
				</parameters>
			</method>
			<method name="get_stroke_color" symbol="champlain_polygon_get_stroke_color">
				<return-type type="ClutterColor*"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
				</parameters>
			</method>
			<method name="get_stroke_width" symbol="champlain_polygon_get_stroke_width">
				<return-type type="gdouble"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
				</parameters>
			</method>
			<method name="hide" symbol="champlain_polygon_hide">
				<return-type type="void"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
				</parameters>
			</method>
			<method name="insert_point" symbol="champlain_polygon_insert_point">
				<return-type type="ChamplainPoint*"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
					<parameter name="lat" type="gdouble"/>
					<parameter name="lon" type="gdouble"/>
					<parameter name="pos" type="gint"/>
				</parameters>
			</method>
			<constructor name="new" symbol="champlain_polygon_new">
				<return-type type="ChamplainPolygon*"/>
			</constructor>
			<method name="remove_point" symbol="champlain_polygon_remove_point">
				<return-type type="void"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
					<parameter name="point" type="ChamplainPoint*"/>
				</parameters>
			</method>
			<method name="set_fill" symbol="champlain_polygon_set_fill">
				<return-type type="void"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
					<parameter name="value" type="gboolean"/>
				</parameters>
			</method>
			<method name="set_fill_color" symbol="champlain_polygon_set_fill_color">
				<return-type type="void"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
					<parameter name="color" type="ClutterColor*"/>
				</parameters>
			</method>
			<method name="set_mark_points" symbol="champlain_polygon_set_mark_points">
				<return-type type="void"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
					<parameter name="value" type="gboolean"/>
				</parameters>
			</method>
			<method name="set_stroke" symbol="champlain_polygon_set_stroke">
				<return-type type="void"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
					<parameter name="value" type="gboolean"/>
				</parameters>
			</method>
			<method name="set_stroke_color" symbol="champlain_polygon_set_stroke_color">
				<return-type type="void"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
					<parameter name="color" type="ClutterColor*"/>
				</parameters>
			</method>
			<method name="set_stroke_width" symbol="champlain_polygon_set_stroke_width">
				<return-type type="void"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
					<parameter name="value" type="gdouble"/>
				</parameters>
			</method>
			<method name="show" symbol="champlain_polygon_show">
				<return-type type="void"/>
				<parameters>
					<parameter name="polygon" type="ChamplainPolygon*"/>
				</parameters>
			</method>
			<property name="closed-path" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="fill" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="fill-color" type="ClutterColor*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="mark-points" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="stroke" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="stroke-color" type="ClutterColor*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="stroke-width" type="gdouble" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="visible" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
		</object>
		<object name="ChamplainSelectionLayer" parent="ChamplainLayer" type-name="ChamplainSelectionLayer" get-type="champlain_selection_layer_get_type">
			<implements>
				<interface name="ClutterScriptable"/>
				<interface name="ClutterContainer"/>
			</implements>
			<method name="count_selected_markers" symbol="champlain_selection_layer_count_selected_markers">
				<return-type type="guint"/>
				<parameters>
					<parameter name="layer" type="ChamplainSelectionLayer*"/>
				</parameters>
			</method>
			<method name="get_selected" symbol="champlain_selection_layer_get_selected">
				<return-type type="ChamplainBaseMarker*"/>
				<parameters>
					<parameter name="layer" type="ChamplainSelectionLayer*"/>
				</parameters>
			</method>
			<method name="get_selected_markers" symbol="champlain_selection_layer_get_selected_markers">
				<return-type type="GList*"/>
				<parameters>
					<parameter name="layer" type="ChamplainSelectionLayer*"/>
				</parameters>
			</method>
			<method name="get_selection_mode" symbol="champlain_selection_layer_get_selection_mode">
				<return-type type="ChamplainSelectionMode"/>
				<parameters>
					<parameter name="layer" type="ChamplainSelectionLayer*"/>
				</parameters>
			</method>
			<method name="marker_is_selected" symbol="champlain_selection_layer_marker_is_selected">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="layer" type="ChamplainSelectionLayer*"/>
					<parameter name="marker" type="ChamplainBaseMarker*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="champlain_selection_layer_new">
				<return-type type="ChamplainLayer*"/>
			</constructor>
			<method name="select" symbol="champlain_selection_layer_select">
				<return-type type="void"/>
				<parameters>
					<parameter name="layer" type="ChamplainSelectionLayer*"/>
					<parameter name="marker" type="ChamplainBaseMarker*"/>
				</parameters>
			</method>
			<method name="select_all" symbol="champlain_selection_layer_select_all">
				<return-type type="void"/>
				<parameters>
					<parameter name="layer" type="ChamplainSelectionLayer*"/>
				</parameters>
			</method>
			<method name="set_selection_mode" symbol="champlain_selection_layer_set_selection_mode">
				<return-type type="void"/>
				<parameters>
					<parameter name="layer" type="ChamplainSelectionLayer*"/>
					<parameter name="mode" type="ChamplainSelectionMode"/>
				</parameters>
			</method>
			<method name="unselect" symbol="champlain_selection_layer_unselect">
				<return-type type="void"/>
				<parameters>
					<parameter name="layer" type="ChamplainSelectionLayer*"/>
					<parameter name="marker" type="ChamplainBaseMarker*"/>
				</parameters>
			</method>
			<method name="unselect_all" symbol="champlain_selection_layer_unselect_all">
				<return-type type="void"/>
				<parameters>
					<parameter name="layer" type="ChamplainSelectionLayer*"/>
				</parameters>
			</method>
			<property name="selection-mode" type="ChamplainSelectionMode" readable="1" writable="1" construct="0" construct-only="0"/>
			<signal name="changed" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="ChamplainSelectionLayer*"/>
				</parameters>
			</signal>
		</object>
		<object name="ChamplainTile" parent="ClutterGroup" type-name="ChamplainTile" get-type="champlain_tile_get_type">
			<implements>
				<interface name="ClutterScriptable"/>
				<interface name="ClutterContainer"/>
			</implements>
			<method name="get_content" symbol="champlain_tile_get_content">
				<return-type type="ClutterActor*"/>
				<parameters>
					<parameter name="self" type="ChamplainTile*"/>
				</parameters>
			</method>
			<method name="get_etag" symbol="champlain_tile_get_etag">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="self" type="ChamplainTile*"/>
				</parameters>
			</method>
			<method name="get_fade_in" symbol="champlain_tile_get_fade_in">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="self" type="ChamplainTile*"/>
				</parameters>
			</method>
			<method name="get_modified_time" symbol="champlain_tile_get_modified_time">
				<return-type type="GTimeVal*"/>
				<parameters>
					<parameter name="self" type="ChamplainTile*"/>
				</parameters>
			</method>
			<method name="get_size" symbol="champlain_tile_get_size">
				<return-type type="guint"/>
				<parameters>
					<parameter name="self" type="ChamplainTile*"/>
				</parameters>
			</method>
			<method name="get_state" symbol="champlain_tile_get_state">
				<return-type type="ChamplainState"/>
				<parameters>
					<parameter name="self" type="ChamplainTile*"/>
				</parameters>
			</method>
			<method name="get_x" symbol="champlain_tile_get_x">
				<return-type type="gint"/>
				<parameters>
					<parameter name="self" type="ChamplainTile*"/>
				</parameters>
			</method>
			<method name="get_y" symbol="champlain_tile_get_y">
				<return-type type="gint"/>
				<parameters>
					<parameter name="self" type="ChamplainTile*"/>
				</parameters>
			</method>
			<method name="get_zoom_level" symbol="champlain_tile_get_zoom_level">
				<return-type type="gint"/>
				<parameters>
					<parameter name="self" type="ChamplainTile*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="champlain_tile_new">
				<return-type type="ChamplainTile*"/>
			</constructor>
			<constructor name="new_full" symbol="champlain_tile_new_full">
				<return-type type="ChamplainTile*"/>
				<parameters>
					<parameter name="x" type="gint"/>
					<parameter name="y" type="gint"/>
					<parameter name="size" type="guint"/>
					<parameter name="zoom_level" type="gint"/>
				</parameters>
			</constructor>
			<method name="set_content" symbol="champlain_tile_set_content">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="ChamplainTile*"/>
					<parameter name="actor" type="ClutterActor*"/>
				</parameters>
			</method>
			<method name="set_etag" symbol="champlain_tile_set_etag">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="ChamplainTile*"/>
					<parameter name="etag" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_fade_in" symbol="champlain_tile_set_fade_in">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="ChamplainTile*"/>
					<parameter name="fade_in" type="gboolean"/>
				</parameters>
			</method>
			<method name="set_modified_time" symbol="champlain_tile_set_modified_time">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="ChamplainTile*"/>
					<parameter name="time" type="GTimeVal*"/>
				</parameters>
			</method>
			<method name="set_size" symbol="champlain_tile_set_size">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="ChamplainTile*"/>
					<parameter name="size" type="guint"/>
				</parameters>
			</method>
			<method name="set_state" symbol="champlain_tile_set_state">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="ChamplainTile*"/>
					<parameter name="state" type="ChamplainState"/>
				</parameters>
			</method>
			<method name="set_x" symbol="champlain_tile_set_x">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="ChamplainTile*"/>
					<parameter name="x" type="gint"/>
				</parameters>
			</method>
			<method name="set_y" symbol="champlain_tile_set_y">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="ChamplainTile*"/>
					<parameter name="y" type="gint"/>
				</parameters>
			</method>
			<method name="set_zoom_level" symbol="champlain_tile_set_zoom_level">
				<return-type type="void"/>
				<parameters>
					<parameter name="self" type="ChamplainTile*"/>
					<parameter name="zoom_level" type="gint"/>
				</parameters>
			</method>
			<property name="content" type="ClutterActor*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="etag" type="char*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="fade-in" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="size" type="guint" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="state" type="ChamplainState" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="x" type="gint" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="y" type="gint" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="zoom-level" type="gint" readable="1" writable="1" construct="0" construct-only="0"/>
		</object>
		<object name="ChamplainTileCache" parent="ChamplainMapSource" type-name="ChamplainTileCache" get-type="champlain_tile_cache_get_type">
			<method name="clean" symbol="champlain_tile_cache_clean">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_cache" type="ChamplainTileCache*"/>
				</parameters>
			</method>
			<method name="get_persistent" symbol="champlain_tile_cache_get_persistent">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="tile_cache" type="ChamplainTileCache*"/>
				</parameters>
			</method>
			<method name="on_tile_filled" symbol="champlain_tile_cache_on_tile_filled">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_cache" type="ChamplainTileCache*"/>
					<parameter name="tile" type="ChamplainTile*"/>
				</parameters>
			</method>
			<method name="refresh_tile_time" symbol="champlain_tile_cache_refresh_tile_time">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_cache" type="ChamplainTileCache*"/>
					<parameter name="tile" type="ChamplainTile*"/>
				</parameters>
			</method>
			<method name="store_tile" symbol="champlain_tile_cache_store_tile">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_cache" type="ChamplainTileCache*"/>
					<parameter name="tile" type="ChamplainTile*"/>
					<parameter name="contents" type="gchar*"/>
					<parameter name="size" type="gsize"/>
				</parameters>
			</method>
			<property name="persistent-cache" type="gboolean" readable="1" writable="1" construct="0" construct-only="1"/>
			<vfunc name="clean">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_cache" type="ChamplainTileCache*"/>
				</parameters>
			</vfunc>
			<vfunc name="on_tile_filled">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_cache" type="ChamplainTileCache*"/>
					<parameter name="tile" type="ChamplainTile*"/>
				</parameters>
			</vfunc>
			<vfunc name="refresh_tile_time">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_cache" type="ChamplainTileCache*"/>
					<parameter name="tile" type="ChamplainTile*"/>
				</parameters>
			</vfunc>
			<vfunc name="store_tile">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_cache" type="ChamplainTileCache*"/>
					<parameter name="tile" type="ChamplainTile*"/>
					<parameter name="contents" type="gchar*"/>
					<parameter name="size" type="gsize"/>
				</parameters>
			</vfunc>
		</object>
		<object name="ChamplainTileSource" parent="ChamplainMapSource" type-name="ChamplainTileSource" get-type="champlain_tile_source_get_type">
			<method name="get_cache" symbol="champlain_tile_source_get_cache">
				<return-type type="ChamplainTileCache*"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainTileSource*"/>
				</parameters>
			</method>
			<method name="set_cache" symbol="champlain_tile_source_set_cache">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainTileSource*"/>
					<parameter name="cache" type="ChamplainTileCache*"/>
				</parameters>
			</method>
			<method name="set_id" symbol="champlain_tile_source_set_id">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainTileSource*"/>
					<parameter name="id" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_license" symbol="champlain_tile_source_set_license">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainTileSource*"/>
					<parameter name="license" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_license_uri" symbol="champlain_tile_source_set_license_uri">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainTileSource*"/>
					<parameter name="license_uri" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_max_zoom_level" symbol="champlain_tile_source_set_max_zoom_level">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainTileSource*"/>
					<parameter name="zoom_level" type="guint"/>
				</parameters>
			</method>
			<method name="set_min_zoom_level" symbol="champlain_tile_source_set_min_zoom_level">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainTileSource*"/>
					<parameter name="zoom_level" type="guint"/>
				</parameters>
			</method>
			<method name="set_name" symbol="champlain_tile_source_set_name">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainTileSource*"/>
					<parameter name="name" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_projection" symbol="champlain_tile_source_set_projection">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainTileSource*"/>
					<parameter name="projection" type="ChamplainMapProjection"/>
				</parameters>
			</method>
			<method name="set_tile_size" symbol="champlain_tile_source_set_tile_size">
				<return-type type="void"/>
				<parameters>
					<parameter name="tile_source" type="ChamplainTileSource*"/>
					<parameter name="tile_size" type="guint"/>
				</parameters>
			</method>
			<property name="cache" type="ChamplainTileCache*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="id" type="char*" readable="1" writable="1" construct="1" construct-only="0"/>
			<property name="license" type="char*" readable="1" writable="1" construct="1" construct-only="0"/>
			<property name="license-uri" type="char*" readable="1" writable="1" construct="1" construct-only="0"/>
			<property name="max-zoom-level" type="guint" readable="1" writable="1" construct="1" construct-only="0"/>
			<property name="min-zoom-level" type="guint" readable="1" writable="1" construct="1" construct-only="0"/>
			<property name="name" type="char*" readable="1" writable="1" construct="1" construct-only="0"/>
			<property name="projection" type="ChamplainMapProjection" readable="1" writable="1" construct="1" construct-only="0"/>
			<property name="tile-size" type="guint" readable="1" writable="1" construct="1" construct-only="0"/>
		</object>
		<object name="ChamplainView" parent="ClutterGroup" type-name="ChamplainView" get-type="champlain_view_get_type">
			<implements>
				<interface name="ClutterScriptable"/>
				<interface name="ClutterContainer"/>
			</implements>
			<method name="add_layer" symbol="champlain_view_add_layer">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="layer" type="ChamplainLayer*"/>
				</parameters>
			</method>
			<method name="add_polygon" symbol="champlain_view_add_polygon">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="polygon" type="ChamplainPolygon*"/>
				</parameters>
			</method>
			<method name="center_on" symbol="champlain_view_center_on">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="latitude" type="gdouble"/>
					<parameter name="longitude" type="gdouble"/>
				</parameters>
			</method>
			<method name="ensure_markers_visible" symbol="champlain_view_ensure_markers_visible">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="markers" type="ChamplainBaseMarker*[]"/>
					<parameter name="animate" type="gboolean"/>
				</parameters>
			</method>
			<method name="ensure_visible" symbol="champlain_view_ensure_visible">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="lat1" type="gdouble"/>
					<parameter name="lon1" type="gdouble"/>
					<parameter name="lat2" type="gdouble"/>
					<parameter name="lon2" type="gdouble"/>
					<parameter name="animate" type="gboolean"/>
				</parameters>
			</method>
			<method name="get_coords_at" symbol="champlain_view_get_coords_at">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="x" type="guint"/>
					<parameter name="y" type="guint"/>
					<parameter name="lat" type="gdouble*"/>
					<parameter name="lon" type="gdouble*"/>
				</parameters>
			</method>
			<method name="get_coords_from_event" symbol="champlain_view_get_coords_from_event">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="event" type="ClutterEvent*"/>
					<parameter name="lat" type="gdouble*"/>
					<parameter name="lon" type="gdouble*"/>
				</parameters>
			</method>
			<method name="get_decel_rate" symbol="champlain_view_get_decel_rate">
				<return-type type="gdouble"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
				</parameters>
			</method>
			<method name="get_keep_center_on_resize" symbol="champlain_view_get_keep_center_on_resize">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
				</parameters>
			</method>
			<method name="get_license_text" symbol="champlain_view_get_license_text">
				<return-type type="gchar*"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
				</parameters>
			</method>
			<method name="get_map_source" symbol="champlain_view_get_map_source">
				<return-type type="ChamplainMapSource*"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
				</parameters>
			</method>
			<method name="get_max_scale_width" symbol="champlain_view_get_max_scale_width">
				<return-type type="guint"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
				</parameters>
			</method>
			<method name="get_max_zoom_level" symbol="champlain_view_get_max_zoom_level">
				<return-type type="gint"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
				</parameters>
			</method>
			<method name="get_min_zoom_level" symbol="champlain_view_get_min_zoom_level">
				<return-type type="gint"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
				</parameters>
			</method>
			<method name="get_scale_unit" symbol="champlain_view_get_scale_unit">
				<return-type type="ChamplainUnit"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
				</parameters>
			</method>
			<method name="get_scroll_mode" symbol="champlain_view_get_scroll_mode">
				<return-type type="ChamplainScrollMode"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
				</parameters>
			</method>
			<method name="get_show_license" symbol="champlain_view_get_show_license">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
				</parameters>
			</method>
			<method name="get_show_scale" symbol="champlain_view_get_show_scale">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
				</parameters>
			</method>
			<method name="get_zoom_level" symbol="champlain_view_get_zoom_level">
				<return-type type="gint"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
				</parameters>
			</method>
			<method name="get_zoom_on_double_click" symbol="champlain_view_get_zoom_on_double_click">
				<return-type type="gboolean"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
				</parameters>
			</method>
			<method name="go_to" symbol="champlain_view_go_to">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="latitude" type="gdouble"/>
					<parameter name="longitude" type="gdouble"/>
				</parameters>
			</method>
			<constructor name="new" symbol="champlain_view_new">
				<return-type type="ClutterActor*"/>
			</constructor>
			<method name="remove_layer" symbol="champlain_view_remove_layer">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="layer" type="ChamplainLayer*"/>
				</parameters>
			</method>
			<method name="remove_polygon" symbol="champlain_view_remove_polygon">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="polygon" type="ChamplainPolygon*"/>
				</parameters>
			</method>
			<method name="set_decel_rate" symbol="champlain_view_set_decel_rate">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="rate" type="gdouble"/>
				</parameters>
			</method>
			<method name="set_keep_center_on_resize" symbol="champlain_view_set_keep_center_on_resize">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="value" type="gboolean"/>
				</parameters>
			</method>
			<method name="set_license_text" symbol="champlain_view_set_license_text">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="text" type="gchar*"/>
				</parameters>
			</method>
			<method name="set_map_source" symbol="champlain_view_set_map_source">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="map_source" type="ChamplainMapSource*"/>
				</parameters>
			</method>
			<method name="set_max_scale_width" symbol="champlain_view_set_max_scale_width">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="value" type="guint"/>
				</parameters>
			</method>
			<method name="set_max_zoom_level" symbol="champlain_view_set_max_zoom_level">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="zoom_level" type="gint"/>
				</parameters>
			</method>
			<method name="set_min_zoom_level" symbol="champlain_view_set_min_zoom_level">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="zoom_level" type="gint"/>
				</parameters>
			</method>
			<method name="set_scale_unit" symbol="champlain_view_set_scale_unit">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="unit" type="ChamplainUnit"/>
				</parameters>
			</method>
			<method name="set_scroll_mode" symbol="champlain_view_set_scroll_mode">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="mode" type="ChamplainScrollMode"/>
				</parameters>
			</method>
			<method name="set_show_license" symbol="champlain_view_set_show_license">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="value" type="gboolean"/>
				</parameters>
			</method>
			<method name="set_show_scale" symbol="champlain_view_set_show_scale">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="value" type="gboolean"/>
				</parameters>
			</method>
			<method name="set_size" symbol="champlain_view_set_size">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="width" type="guint"/>
					<parameter name="height" type="guint"/>
				</parameters>
			</method>
			<method name="set_zoom_level" symbol="champlain_view_set_zoom_level">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="zoom_level" type="gint"/>
				</parameters>
			</method>
			<method name="set_zoom_on_double_click" symbol="champlain_view_set_zoom_on_double_click">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
					<parameter name="value" type="gboolean"/>
				</parameters>
			</method>
			<method name="stop_go_to" symbol="champlain_view_stop_go_to">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
				</parameters>
			</method>
			<method name="zoom_in" symbol="champlain_view_zoom_in">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
				</parameters>
			</method>
			<method name="zoom_out" symbol="champlain_view_zoom_out">
				<return-type type="void"/>
				<parameters>
					<parameter name="view" type="ChamplainView*"/>
				</parameters>
			</method>
			<property name="decel-rate" type="gdouble" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="keep-center-on-resize" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="latitude" type="gdouble" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="license-text" type="char*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="longitude" type="gdouble" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="map-source" type="ChamplainMapSource*" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="max-scale-width" type="guint" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="max-zoom-level" type="gint" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="min-zoom-level" type="gint" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="scale-unit" type="ChamplainUnit" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="scroll-mode" type="ChamplainScrollMode" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="show-license" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="show-scale" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="state" type="ChamplainState" readable="1" writable="0" construct="0" construct-only="0"/>
			<property name="zoom-level" type="gint" readable="1" writable="1" construct="0" construct-only="0"/>
			<property name="zoom-on-double-click" type="gboolean" readable="1" writable="1" construct="0" construct-only="0"/>
			<signal name="animation-completed" when="LAST">
				<return-type type="void"/>
				<parameters>
					<parameter name="object" type="ChamplainView*"/>
				</parameters>
			</signal>
		</object>
		<constant name="CHAMPLAIN_HAS_MEMPHIS" type="int" value="1"/>
		<constant name="CHAMPLAIN_MAJOR_VERSION" type="int" value="0"/>
		<constant name="CHAMPLAIN_MAP_SOURCE_MEMPHIS_LOCAL" type="char*" value="memphis-local"/>
		<constant name="CHAMPLAIN_MAP_SOURCE_MEMPHIS_NETWORK" type="char*" value="memphis-network"/>
		<constant name="CHAMPLAIN_MAP_SOURCE_MFF_RELIEF" type="char*" value="mff-relief"/>
		<constant name="CHAMPLAIN_MAP_SOURCE_OAM" type="char*" value="OpenAerialMap"/>
		<constant name="CHAMPLAIN_MAP_SOURCE_OSM_CYCLE_MAP" type="char*" value="osm-cyclemap"/>
		<constant name="CHAMPLAIN_MAP_SOURCE_OSM_MAPNIK" type="char*" value="osm-mapnik"/>
		<constant name="CHAMPLAIN_MAP_SOURCE_OSM_OSMARENDER" type="char*" value="osm-osmarender"/>
		<constant name="CHAMPLAIN_MAP_SOURCE_OSM_TRANSPORT_MAP" type="char*" value="osm-transportmap"/>
		<constant name="CHAMPLAIN_MICRO_VERSION" type="int" value="0"/>
		<constant name="CHAMPLAIN_MINOR_VERSION" type="int" value="6"/>
		<constant name="CHAMPLAIN_VERSION_HEX" type="int" value="0"/>
		<constant name="CHAMPLAIN_VERSION_S" type="char*" value="0.6.0"/>
	</namespace>
</api>

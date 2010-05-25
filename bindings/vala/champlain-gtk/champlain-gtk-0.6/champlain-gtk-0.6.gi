<?xml version="1.0"?>
<api version="1.0">
	<namespace name="GtkChamplain">
		<object name="GtkChamplainEmbed" parent="GtkAlignment" type-name="GtkChamplainEmbed" get-type="gtk_champlain_embed_get_type">
			<implements>
				<interface name="AtkImplementor"/>
				<interface name="GtkBuildable"/>
			</implements>
			<method name="get_view" symbol="gtk_champlain_embed_get_view">
				<return-type type="ChamplainView*"/>
				<parameters>
					<parameter name="embed" type="GtkChamplainEmbed*"/>
				</parameters>
			</method>
			<constructor name="new" symbol="gtk_champlain_embed_new">
				<return-type type="GtkWidget*"/>
			</constructor>
			<property name="champlain-view" type="ChamplainView*" readable="1" writable="0" construct="0" construct-only="0"/>
		</object>
	</namespace>
</api>

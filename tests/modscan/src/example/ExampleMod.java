package example;

import net.neoforged.api.distmarker.Dist;
import net.neoforged.bus.api.EventPriority;
import net.neoforged.bus.api.SubscribeEvent;
import net.neoforged.fml.common.EventBusSubscriber;
import net.neoforged.fml.common.Mod;

@Mod(value = "examplemod", dist = { Dist.CLIENT }, depends = { "othermod", "thirdmod" })
@Complex(count = 7, flag = false, choice = Complex.Choice.B, tags = { "x", "y" }, nested = @Complex.Nested(name = "inner"))
public class ExampleMod implements Runnable {
    @SubscribeEvent
    public static void onSomething(String event) {
    }

    @SubscribeEvent(priority = EventPriority.HIGH)
    @Deprecated
    public int counter = 1;

    @Override
    public void run() {
    }
}

@EventBusSubscriber(modid = "examplemod")
class ExampleSubscriber {
    @SubscribeEvent(priority = EventPriority.HIGHEST, receiveCanceled = true)
    private static void onOther(String event) {
    }
}

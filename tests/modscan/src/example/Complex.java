package example;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
public @interface Complex {
    enum Choice {
        A,
        B
    }

    int count() default 3;

    boolean flag() default true;

    Choice choice() default Choice.A;

    String[] tags() default { "x", "y" };

    Nested nested();

    @Retention(RetentionPolicy.RUNTIME)
    @interface Nested {
        String name();
    }
}

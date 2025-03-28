/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 *
 * This file includes only framework internal classes and functions
 * non are part of SDK. Do not access from app.
 *
 *
 * Helper class for handling V2 decorated variables
 */
class VariableUtilV3 {
    /**
       * setReadOnlyAttr - helper function used to update @param
       * from parent @Component. Not allowed for @param @once .
       * @param target  - the object, usually the ViewV2
       * @param attrName - @param variable name
       * @param newValue - update to new value
       */
    public static initParam<Z>(target: object, attrName: string, newValue: Z): void {
      const meta = target[ObserveV2.V2_DECO_META]?.[attrName];
      if (!meta || meta.deco !== '@param') {
        const error = `Use initParam(${attrName}) only to init @param. Internal error!`;
        stateMgmtConsole.error(error);
        throw new Error(error);
      }
      // prevent update for @param @once
      const storeProp = ObserveV2.OB_PREFIX + attrName;
      stateMgmtConsole.propertyAccess(`initParam '@param ${attrName}' - setting backing store`);
      target[storeProp] = newValue;
      ObserveV2.getObserve().addRef(target, attrName);
    }

      /**
       * setReadOnlyAttr - helper function used to update @param
       * from parent @Component. Not allowed for @param @once .
       * @param target  - the object, usually the ViewV2
       * @param attrName - @param variable name
       * @param newValue - update to new value
       */
    public static updateParam<Z>(target: object, attrName: string, newValue: Z): void {
      // prevent update for @param @once
      const meta = target[ObserveV2.V2_DECO_META]?.[attrName];
      if (!meta || meta.deco !== '@param') {
        const error = `Use updateParm(${attrName}) only to update @param. Internal error!`;
        stateMgmtConsole.error(error);
        throw new Error(error);
      }

      const storeProp = ObserveV2.OB_PREFIX + attrName;
      // @observed class and @track attrName
      if (newValue === target[storeProp]) {
        stateMgmtConsole.propertyAccess(`updateParm '@param ${attrName}' unchanged. Doing nothing.`);
        return;
      }
      if (meta.deco2 === '@once') {
        // @param @once - init but no update
        stateMgmtConsole.log(`updateParm: '@param @once ${attrName}' - Skip updating.`);
      } else {
        stateMgmtConsole.propertyAccess(`updateParm '@param ${attrName}' - updating backing store and fireChange.`);
        target[storeProp] = newValue;
        ObserveV2.getObserve().fireChange(target, attrName);
      }
    }
  }

  class ProviderConsumerUtilV2 {
    private static readonly ALIAS_PREFIX = '___pc_alias_';

    /**
     *  meta added to the ViewV2
     *  varName: { deco: '@Provider' | '@Consumer', aliasName: ..... }
     *  prefix_@Provider_aliasName: {'varName': ..., 'aliasName': ...., 'deco': '@Provider' | '@Consumer'
     */
    private static metaAliasKey(aliasName: string, deco: '@Provider' | '@Consumer') : string {
      return `${ProviderConsumerUtilV2.ALIAS_PREFIX}_${deco}_${aliasName}`;;
    }

    /**
     * Helper function to add meta data about @Provider and @Consumer decorators to ViewV2
     * similar to @see addVariableDecoMeta, but adds the alias to allow search from @Consumer for @Provider counterpart
     * @param proto prototype object of application class derived from ViewV2
     * @param varName decorated variable
     * @param deco '@state', '@event', etc (note '@model' gets transpiled in '@param' and '@event')
     */
    public static addProvideConsumeVariableDecoMeta(proto: Object, varName: string, aliasName: string, deco: '@Provider' | '@Consumer'): void {
      // add decorator meta data to prototype
      const meta = proto[ObserveV2.V2_DECO_META] ??= {};
      // note: aliasName is the actual alias not the prefixed version
      meta[varName] = { 'deco': deco, 'aliasName': aliasName };

      // prefix to avoid name collisions with variable of same name as the alias!
      const aliasProp = ProviderConsumerUtilV2.metaAliasKey(aliasName, deco);
      meta[aliasProp] = { 'varName': varName, 'aliasName': aliasName, 'deco': deco };
    }

    public static setupConsumeVarsV2(view: ViewV2): boolean {
      const meta = view && view[ObserveV2.V2_DECO_META];
      if (!meta) {
        return;
      }

      for (const [key, value] of Object.entries(meta)) {
        // check all entries of this format varName: { deco: '@Provider' | '@Consumer', aliasName: ..... }
        // do not check alias entries
        // 'varName' is only in alias entries, see addProvideConsumeVariableDecoMeta
        if (typeof value == 'object' && value['deco'] === '@Consumer' && !('varName' in value)) {
          let result = ProviderConsumerUtilV2.findProvider(view, value['aliasName']);
          if (result && result[0] && result[1]) {
            ProviderConsumerUtilV2.connectConsumer2Provider(view, key, result[0], result[1]);
          } else {
            ProviderConsumerUtilV2.defineConsumerWithoutProvider(view, key);
          }
        }
      }
    }

    /**
    * find a @Provider'ed variable from its nearest ancestor ViewV2.
    * @param searchingAliasName The key name to search for.
    * @returns A tuple containing the ViewPU instance where the provider is found
    * and the provider name
    * If root @Component reached without finding, returns undefined.
    */
    private static findProvider(view: ViewV2, aliasName: string): [ViewV2, string] | undefined {
      let checkView : IView | undefined = view?.getParent();
      const searchingPrefixedAliasName = ProviderConsumerUtilV2.metaAliasKey(aliasName, "@Provider");
      stateMgmtConsole.debug(`findProvider: Try to connect ${view.debugInfo__()} '@Consumer ${aliasName}' to @Provider counterpart....`);
      while (checkView) {
        const meta = checkView.constructor?.prototype[ObserveV2.V2_DECO_META];
        if (checkView instanceof ViewV2 && meta && meta[searchingPrefixedAliasName]) {
          const aliasMeta = meta[searchingPrefixedAliasName];
          const providedVarName: string | undefined = (aliasMeta && (aliasMeta.deco === '@Provider') ? aliasMeta.varName : undefined);

          if (providedVarName) {
            stateMgmtConsole.debug(`findProvider: success: ${checkView.debugInfo__()} has matching @Provider('${aliasName}') ${providedVarName}`);
            return [checkView, providedVarName];
          }
        }
        checkView = checkView.getParent();
      }; // while
      stateMgmtConsole.warn(`findProvider: ${view.debugInfo__()} @Consumer('${aliasName}'), no matching @Provider found amongst ancestor @ComponentV2's!`);
      return undefined;
    }

    private static connectConsumer2Provider(consumeView: ViewV2, consumeVarName: string, provideView: ViewV2, provideVarName: string): void {
      const weakView = new WeakRef<ViewV2>(provideView);
      const provideViewName = provideView.constructor?.name;
      Reflect.defineProperty(consumeView, consumeVarName, {
        get() {
          stateMgmtConsole.propertyAccess(`@Consumer ${consumeVarName} get`);
          ObserveV2.getObserve().addRef(this, consumeVarName);
          const view = weakView.deref();
          if (!view) {
            const error = `${this.debugInfo__()}: get() on @Consumer ${consumeVarName}: providing @ComponentV2 with @Provider ${provideViewName} no longer exists. Application error.`;
            stateMgmtConsole.error(error);
            throw new Error(error);
          }
          return view[provideVarName];
        },
        set(val) {
          // If the object has not been observed, you can directly assign a value to it. This improves performance.
          stateMgmtConsole.propertyAccess(`@Consumer ${consumeVarName} set`);
          const view = weakView.deref();
          if (!view) {
            const error = `${this.debugInfo__()}: set() on @Consumer ${consumeVarName}: providing @ComponentV2 with @Provider ${provideViewName} no longer exists. Application error.`;
            stateMgmtConsole.error(error);
            throw new Error(error);
          }

          if (val !== view[provideVarName]) {
            stateMgmtConsole.propertyAccess(`@Consumer ${consumeVarName} valueChanged`);
            view[provideVarName] = val;
            if (this[ObserveV2.SYMBOL_REFS]) { // This condition can improve performance.
              ObserveV2.getObserve().fireChange(this, consumeVarName);
            }
          }
        },
        enumerable: true
      });
    }

    private static defineConsumerWithoutProvider(consumeView: ViewV2, consumeVarName: string): void {
      stateMgmtConsole.debug(`defineConsumerWithoutProvider: ${consumeView.debugInfo__()} @Consumer ${consumeVarName} does not have @Provider counter part, will use local init value`);

      const storeProp = ObserveV2.OB_PREFIX + consumeVarName;
      consumeView[storeProp] = consumeView[consumeVarName]; // use local init value, also as backing store
      Reflect.defineProperty(consumeView, consumeVarName, {
        get() {
          ObserveV2.getObserve().addRef(this, consumeVarName);
          return ObserveV2.autoProxyObject(this, ObserveV2.OB_PREFIX + consumeVarName);
        },
        set(val) {
          if (val !== this[storeProp]) {
            this[storeProp] = val;
            if (this[ObserveV2.SYMBOL_REFS]) { // This condition can improve performance.
              ObserveV2.getObserve().fireChange(this, consumeVarName);
            }
          }
        },
        enumerable: true
      });
    }
  }

/*
  Internal decorator for @Trace without usingV2ObservedTrack call.
  Real @Trace decorator function is in v2_decorators.ts
*/
const Trace_Internal = (target: Object, propertyKey: string): void => {
    return trackInternal(target, propertyKey);
};

/*
  Internal decorator for @ObservedV2 without usingV2ObservedTrack call.
  Real @ObservedV2 decorator function is in v2_decorators.ts
*/
function ObservedV2_Internal<T extends ConstructorV2>(BaseClass: T): T {
    return observedV2Internal<T>(BaseClass);
}

/*
  @ObservedV2 decorator function uses this in v2_decorators.ts
*/
function observedV2Internal<T extends ConstructorV2>(BaseClass: T): T {

  // prevent @Track inside @observed class
  if (BaseClass.prototype && Reflect.has(BaseClass.prototype, TrackedObject.___IS_TRACKED_OPTIMISED)) {
    const error = `'@observed class ${BaseClass?.name}': invalid use of V2 @Track decorator inside V3 @observed class. Need to fix class definition to use @track.`;
    stateMgmtConsole.applicationError(error);
    throw new Error(error);
  }

  if (BaseClass.prototype && !Reflect.has(BaseClass.prototype, ObserveV2.V2_DECO_META)) {
    // not an error, suspicious of developer oversight
    stateMgmtConsole.warn(`'@observed class ${BaseClass?.name}': no @track property inside. Is this intended? Check our application.`);
  }

  // Use ID_REFS only if number of observed attrs is significant
  const attrList = Object.getOwnPropertyNames(BaseClass.prototype);
  const count = attrList.filter(attr => attr.startsWith(ObserveV2.OB_PREFIX)).length;
  if (count > 5) {
    stateMgmtConsole.log(`'@observed class ${BaseClass?.name}' configured to use ID_REFS optimization`);
    BaseClass.prototype[ObserveV2.ID_REFS] = {};
  }
  const observedClass =  class extends BaseClass {
    constructor(...args) {
      super(...args);
      AsyncAddComputedV2.addComputed(this, BaseClass.name);
      AsyncAddMonitorV2.addMonitor(this, BaseClass.name);
    }
  };
  Object.defineProperty(observedClass, 'name', { value: BaseClass.name });
  return observedClass;
}
